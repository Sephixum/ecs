#include <cassert>
#include <cstdio>
#include <ecs/ecs.hpp>
#include <vector>

// ──────────────────────────────────────────
// Components
// ──────────────────────────────────────────

struct Position
{
	float x = 0.f, y = 0.f;
};
struct Velocity
{
	float x = 0.f, y = 0.f;
};
struct Health
{
	float value = 100.f;
};
struct Tag
{
};

// ──────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────

#define TEST(name) std::printf("  %-50s", #name);
#define PASS() std::printf("PASS\n");
#define FAIL(msg)                                                                                                      \
	std::printf("FAIL — %s\n", msg);                                                                                   \
	std::exit(1);
#define EXPECT(expr)                                                                                                   \
	if (!(expr))                                                                                                       \
	{                                                                                                                  \
		FAIL(#expr)                                                                                                    \
	}

// ──────────────────────────────────────────
// System Definitions
// ──────────────────────────────────────────

class MovementSystem : public ecs::ISystem<Position, Velocity>
{
  public:
	MovementSystem(float& dt) : m_dt{dt} {}

	void Update(ecs::View<Position, Velocity> view) override
	{
		view.Each(
			[this](Position& p, Velocity& v)
			{
				p.x += v.x * m_dt;
				p.y += v.y * m_dt;
			});
	}

  private:
	float& m_dt;
};

class HealthDecaySystem : public ecs::ISystem<Health>
{
  public:
	void Update(ecs::View<Health> view) override
	{
		view.Each([](Health& h) { h.value -= 10.f; });
	}
};

class DestroyDeadSystem : public ecs::ISystem<Health>
{
  public:
	void Update(ecs::View<Health> view) override
	{
		view.Each(
			[](ecs::Entity e, Health& h)
			{
				if (h.value <= 0.f)
					e.Destroy();
			});
	}
};

class OnCreateTracker : public ecs::ISystem<Position>
{
  public:
	bool created   = false;
	bool destroyed = false;

	void OnCreate(ecs::World&) override
	{
		created = true;
	}
	void OnDestroy(ecs::World&) override
	{
		destroyed = true;
	}
	void Update(ecs::View<Position>) override {}
};

// ──────────────────────────────────────────
// Entity Tests
// ──────────────────────────────────────────

void test_create_entity()
{
	TEST(create_entity)
	ecs::World world;

	auto e1 = world.CreateEntity();
	auto e2 = world.CreateEntity();
	auto e3 = world.CreateEntity();

	EXPECT(world.IsAlive(e1.GetHandle()));
	EXPECT(world.IsAlive(e2.GetHandle()));
	EXPECT(world.IsAlive(e3.GetHandle()));
	EXPECT(world.GetEntityCount() == 3);
	PASS()
}

void test_destroy_entity()
{
	TEST(destroy_entity)
	ecs::World world;

	auto e1 = world.CreateEntity();
	auto e2 = world.CreateEntity();

	e1.Destroy();

	EXPECT(!world.IsAlive(e1.GetHandle()));
	EXPECT(world.IsAlive(e2.GetHandle()));
	EXPECT(world.GetEntityCount() == 1);
	PASS()
}

void test_stale_handle()
{
	TEST(stale_handle_after_destroy)
	ecs::World world;

	auto e1		= world.CreateEntity();
	auto handle = e1.GetHandle();
	e1.Destroy();

	auto e2 = world.CreateEntity();
	EXPECT(e2.GetHandle().index == handle.index);
	EXPECT(e2.GetHandle().generation != handle.generation);
	EXPECT(!world.IsAlive(handle));
	EXPECT(world.IsAlive(e2.GetHandle()));
	PASS()
}

void test_entity_count()
{
	TEST(entity_count)
	ecs::World world;

	EXPECT(world.GetEntityCount() == 0);

	auto e1 = world.CreateEntity();
	auto e2 = world.CreateEntity();
	auto e3 = world.CreateEntity();
	EXPECT(world.GetEntityCount() == 3);

	e2.Destroy();
	EXPECT(world.GetEntityCount() == 2);

	e1.Destroy();
	e3.Destroy();
	EXPECT(world.GetEntityCount() == 0);
	PASS()
}

// ──────────────────────────────────────────
// Component Tests
// ──────────────────────────────────────────

void test_add_get_component()
{
	TEST(add_and_get_component)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(1.f, 2.f);

	EXPECT(e.GetComponent<Position>().x == 1.f);
	EXPECT(e.GetComponent<Position>().y == 2.f);
	PASS()
}

void test_has_component()
{
	TEST(has_component)
	ecs::World world;

	auto e = world.CreateEntity();
	EXPECT(!e.HasComponent<Position>());

	e.AddComponent<Position>();
	EXPECT(e.HasComponent<Position>());
	PASS()
}

void test_try_get_component()
{
	TEST(try_get_component)
	ecs::World world;

	auto e = world.CreateEntity();
	EXPECT(e.TryGetComponent<Position>() == nullptr);

	e.AddComponent<Position>(3.f, 4.f);
	auto* p = e.TryGetComponent<Position>();
	EXPECT(p != nullptr);
	EXPECT(p->x == 3.f);
	EXPECT(p->y == 4.f);
	PASS()
}

void test_remove_component()
{
	TEST(remove_component)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(1.f, 2.f).AddComponent<Velocity>(3.f, 4.f);

	EXPECT(e.HasComponent<Position>());
	EXPECT(e.HasComponent<Velocity>());

	e.RemoveComponent<Velocity>();

	EXPECT(e.HasComponent<Position>());
	EXPECT(!e.HasComponent<Velocity>());
	EXPECT(e.GetComponent<Position>().x == 1.f);
	EXPECT(e.GetComponent<Position>().y == 2.f);
	PASS()
}

void test_multiple_components()
{
	TEST(multiple_components)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(10.f, 20.f).AddComponent<Velocity>(1.f, 2.f).AddComponent<Health>(50.f);

	EXPECT(e.HasComponent<Position>());
	EXPECT(e.HasComponent<Velocity>());
	EXPECT(e.HasComponent<Health>());

	EXPECT(e.GetComponent<Position>().x == 10.f);
	EXPECT(e.GetComponent<Velocity>().x == 1.f);
	EXPECT(e.GetComponent<Health>().value == 50.f);
	PASS()
}

void test_archetype_migration()
{
	TEST(archetype_migration_preserves_data)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(5.f, 6.f).AddComponent<Velocity>(7.f, 8.f);

	e.AddComponent<Health>(99.f);

	EXPECT(e.GetComponent<Position>().x == 5.f);
	EXPECT(e.GetComponent<Position>().y == 6.f);
	EXPECT(e.GetComponent<Velocity>().x == 7.f);
	EXPECT(e.GetComponent<Velocity>().y == 8.f);
	EXPECT(e.GetComponent<Health>().value == 99.f);
	PASS()
}

void test_swap_remove_correctness()
{
	TEST(swap_remove_entity_record_correctness)
	ecs::World world;

	auto e1 = world.CreateEntity();
	auto e2 = world.CreateEntity();
	auto e3 = world.CreateEntity();

	e1.AddComponent<Position>(1.f, 0.f);
	e2.AddComponent<Position>(2.f, 0.f);
	e3.AddComponent<Position>(3.f, 0.f);

	e2.Destroy();

	EXPECT(e1.IsAlive());
	EXPECT(e3.IsAlive());
	EXPECT(!e2.IsAlive());

	EXPECT(e1.GetComponent<Position>().x == 1.f);
	EXPECT(e3.GetComponent<Position>().x == 3.f);
	PASS()
}

void test_component_add_remove_cycle()
{
	TEST(component_add_remove_cycle)
	ecs::World world;

	auto e = world.CreateEntity();
	for (int i = 0; i < 100; ++i)
	{
		e.AddComponent<Position>((float)i, 0.f);
		EXPECT(e.GetComponent<Position>().x == (float)i);
		e.RemoveComponent<Position>();
		EXPECT(!e.HasComponent<Position>());
	}
	PASS()
}

void test_get_component_throws()
{
	TEST(get_component_throws_on_missing)
	ecs::World world;

	auto e	   = world.CreateEntity();
	bool threw = false;
	try
	{
		e.GetComponent<Position>();
	}
	catch (std::runtime_error const&)
	{
		threw = true;
	}

	EXPECT(threw);
	PASS()
}

// ──────────────────────────────────────────
// Entity API Tests
// ──────────────────────────────────────────

void test_entity_add_get_component()
{
	TEST(entity_add_and_get_component)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(1.f, 2.f);

	EXPECT(e.HasComponent<Position>());
	EXPECT(e.GetComponent<Position>().x == 1.f);
	EXPECT(e.GetComponent<Position>().y == 2.f);
	PASS()
}

void test_entity_chaining()
{
	TEST(entity_component_chaining)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(1.f, 0.f).AddComponent<Velocity>(2.f, 0.f).AddComponent<Health>(100.f);

	EXPECT(e.HasComponent<Position>());
	EXPECT(e.HasComponent<Velocity>());
	EXPECT(e.HasComponent<Health>());
	PASS()
}

void test_entity_remove_component()
{
	TEST(entity_remove_component)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(1.f, 2.f).AddComponent<Velocity>(3.f, 4.f);

	e.RemoveComponent<Velocity>();

	EXPECT(e.HasComponent<Position>());
	EXPECT(!e.HasComponent<Velocity>());
	EXPECT(e.GetComponent<Position>().x == 1.f);
	PASS()
}

void test_entity_try_get_component()
{
	TEST(entity_try_get_component)
	ecs::World world;

	auto e = world.CreateEntity();
	EXPECT(e.TryGetComponent<Position>() == nullptr);

	e.AddComponent<Position>(5.f, 6.f);
	auto* p = e.TryGetComponent<Position>();
	EXPECT(p != nullptr);
	EXPECT(p->x == 5.f);
	PASS()
}

void test_entity_destroy()
{
	TEST(entity_destroy)
	ecs::World world;

	auto e		= world.CreateEntity();
	auto handle = e.GetHandle();

	EXPECT(e.IsAlive());
	e.Destroy();
	EXPECT(!e.IsAlive());
	EXPECT(!world.IsAlive(handle));
	PASS()
}

void test_entity_get_world()
{
	TEST(entity_get_world)
	ecs::World world;

	auto e = world.CreateEntity();
	EXPECT(&e.GetWorld() == &world);
	PASS()
}

void test_entity_handle_conversion()
{
	TEST(entity_implicit_handle_conversion)
	ecs::World world;

	auto			  e		 = world.CreateEntity();
	ecs::EntityHandle handle = e;

	EXPECT(handle == e.GetHandle());
	EXPECT(world.IsAlive(handle));
	PASS()
}

// ──────────────────────────────────────────
// Query / View Tests
// ──────────────────────────────────────────

void test_query_no_entity()
{
	TEST(query_each_without_entity)
	ecs::World world;

	auto e1 = world.CreateEntity();
	auto e2 = world.CreateEntity();

	e1.AddComponent<Position>(1.f, 0.f).AddComponent<Velocity>(1.f, 0.f);
	e2.AddComponent<Position>(2.f, 0.f).AddComponent<Velocity>(2.f, 0.f);

	int count = 0;
	world.Query<Position, Velocity>().Each(
		[&](Position& p, Velocity& v)
		{
			p.x += v.x;
			++count;
		});

	EXPECT(count == 2);
	EXPECT(e1.GetComponent<Position>().x == 2.f);
	EXPECT(e2.GetComponent<Position>().x == 4.f);
	PASS()
}

void test_query_with_entity()
{
	TEST(query_each_with_entity)
	ecs::World world;

	auto e1 = world.CreateEntity();
	e1.AddComponent<Position>(5.f, 0.f).AddComponent<Velocity>(1.f, 0.f);

	ecs::EntityHandle seen = ecs::EntityHandle::Invalid();
	world.Query<Position, Velocity>().Each([&](ecs::Entity e, Position&, Velocity&) { seen = e.GetHandle(); });

	EXPECT(seen == e1.GetHandle());
	PASS()
}

void test_query_superset_archetypes()
{
	TEST(query_matches_superset_archetypes)
	ecs::World world;

	auto e1 = world.CreateEntity();
	e1.AddComponent<Position>(1.f, 0.f).AddComponent<Velocity>(1.f, 0.f);

	auto e2 = world.CreateEntity();
	e2.AddComponent<Position>(2.f, 0.f).AddComponent<Velocity>(1.f, 0.f).AddComponent<Health>(50.f);

	int count = 0;
	world.Query<Position, Velocity>().Each([&](Position&, Velocity&) { ++count; });

	EXPECT(count == 2);
	PASS()
}

void test_view_refresh()
{
	TEST(view_refreshes_after_new_archetype)
	ecs::World world;

	auto view = world.Query<Position, Velocity>();

	auto e = world.CreateEntity();
	e.AddComponent<Position>(1.f, 0.f).AddComponent<Velocity>(1.f, 0.f);

	int count = 0;
	view.Each([&](Position&, Velocity&) { ++count; });

	EXPECT(count == 1);
	PASS()
}

void test_query_empty_world()
{
	TEST(query_each_on_empty_world)
	ecs::World world;

	int count = 0;
	world.Query<Position, Velocity>().Each([&](Position&, Velocity&) { ++count; });

	EXPECT(count == 0);
	PASS()
}

void test_query_no_matching_components()
{
	TEST(query_each_with_no_matching_entities)
	ecs::World world;

	auto e = world.CreateEntity();
	e.AddComponent<Position>(1.f, 0.f);

	int count = 0;
	world.Query<Position, Velocity>().Each([&](Position&, Velocity&) { ++count; });

	EXPECT(count == 0);
	PASS()
}

// ──────────────────────────────────────────
// Stress Tests
// ──────────────────────────────────────────

void test_large_entity_count()
{
	TEST(large_entity_count)
	ecs::World world;

	constexpr int N = 10000;
	for (int i = 0; i < N; ++i)
	{
		world.CreateEntity().AddComponent<Position>((float)i, 0.f).AddComponent<Velocity>(1.f, 0.f);
	}

	int count = 0;
	world.Query<Position, Velocity>().Each(
		[&](Position& p, Velocity& v)
		{
			p.x += v.x;
			++count;
		});

	EXPECT(count == N);
	PASS()
}

void test_mass_destroy()
{
	TEST(mass_destroy_and_query)
	ecs::World world;

	std::vector<ecs::Entity> entities;
	entities.reserve(100);
	for (int i = 0; i < 100; ++i)
	{
		auto e = world.CreateEntity();
		e.AddComponent<Position>((float)i, 0.f);
		entities.push_back(e);
	}

	for (int i = 0; i < 100; i += 2)
		entities[i].Destroy();

	EXPECT(world.GetEntityCount() == 50);

	int count = 0;
	world.Query<Position>().Each([&](Position&) { ++count; });
	EXPECT(count == 50);
	PASS()
}

// ──────────────────────────────────────────
// System Tests
// ──────────────────────────────────────────

void test_system_basic()
{
	TEST(system_basic_update)
	ecs::World world;

	float dt = 1.f;
	world.Register<MovementSystem>(ecs::Stage::Update, dt);

	auto e = world.CreateEntity();
	e.AddComponent<Position>(0.f, 0.f).AddComponent<Velocity>(2.f, 3.f);

	world.RunStage(ecs::Stage::Update);

	EXPECT(e.GetComponent<Position>().x == 2.f);
	EXPECT(e.GetComponent<Position>().y == 3.f);
	PASS()
}

void test_system_delta_time()
{
	TEST(system_delta_time_injection)
	ecs::World world;

	float dt = 0.5f;
	world.Register<MovementSystem>(ecs::Stage::Update, dt);

	auto e = world.CreateEntity();
	e.AddComponent<Position>(0.f, 0.f).AddComponent<Velocity>(4.f, 0.f);

	world.RunStage(ecs::Stage::Update);
	EXPECT(e.GetComponent<Position>().x == 2.f);

	dt = 1.f;
	world.RunStage(ecs::Stage::Update);
	EXPECT(e.GetComponent<Position>().x == 6.f);
	PASS()
}

void test_system_stage_ordering()
{
	TEST(system_stage_ordering)
	ecs::World		 world;
	std::vector<int> order;

	struct StageA : ecs::ISystem<Position>
	{
		StageA(std::vector<int>& o) : m_o{o} {}
		void Update(ecs::View<Position>) override
		{
			m_o.push_back(0);
		}
		std::vector<int>& m_o;
	};
	struct StageB : ecs::ISystem<Position>
	{
		StageB(std::vector<int>& o) : m_o{o} {}
		void Update(ecs::View<Position>) override
		{
			m_o.push_back(1);
		}
		std::vector<int>& m_o;
	};
	struct StageC : ecs::ISystem<Position>
	{
		StageC(std::vector<int>& o) : m_o{o} {}
		void Update(ecs::View<Position>) override
		{
			m_o.push_back(2);
		}
		std::vector<int>& m_o;
	};

	world.Register<StageC>(ecs::Stage::PostUpdate, order);
	world.Register<StageA>(ecs::Stage::PreUpdate, order);
	world.Register<StageB>(ecs::Stage::Update, order);

	world.RunAll();

	EXPECT(order.size() == 3);
	EXPECT(order[0] == 0);
	EXPECT(order[1] == 1);
	EXPECT(order[2] == 2);
	PASS()
}

void test_system_priority()
{
	TEST(system_priority_within_stage)
	ecs::World		 world;
	std::vector<int> order;

	struct SysA : ecs::ISystem<Position>
	{
		SysA(std::vector<int>& o) : m_o{o} {}
		void Update(ecs::View<Position>) override
		{
			m_o.push_back(0);
		}
		std::vector<int>& m_o;
	};
	struct SysB : ecs::ISystem<Position>
	{
		SysB(std::vector<int>& o) : m_o{o} {}
		void Update(ecs::View<Position>) override
		{
			m_o.push_back(1);
		}
		std::vector<int>& m_o;
	};
	struct SysC : ecs::ISystem<Position>
	{
		SysC(std::vector<int>& o) : m_o{o} {}
		void Update(ecs::View<Position>) override
		{
			m_o.push_back(2);
		}
		std::vector<int>& m_o;
	};

	world.Register<SysB>(ecs::Stage::Update, ecs::Priority{0}, order);
	world.Register<SysA>(ecs::Stage::Update, ecs::Priority{-1}, order);
	world.Register<SysC>(ecs::Stage::Update, ecs::Priority{1}, order);

	world.RunStage(ecs::Stage::Update);

	EXPECT(order.size() == 3);
	EXPECT(order[0] == 0);
	EXPECT(order[1] == 1);
	EXPECT(order[2] == 2);
	PASS()
}

void test_system_entity_destroy()
{
	TEST(system_entity_destroy_via_entity)
	ecs::World world;

	world.Register<HealthDecaySystem>(ecs::Stage::Update);
	world.Register<DestroyDeadSystem>(ecs::Stage::Update, ecs::Priority{1});

	for (int i = 0; i < 5; ++i)
	{
		world.CreateEntity().AddComponent<Health>(10.f);
	}

	EXPECT(world.GetEntityCount() == 5);

	// one RunAll drains health to 0 then destroys
	world.RunAll();

	EXPECT(world.GetEntityCount() == 0);
	PASS()
}

void test_system_oncreate_ondestroy()
{
	TEST(system_oncreate_ondestroy_hooks)
	ecs::World world;

	bool created   = false;
	bool destroyed = false;

	struct Tracker : ecs::ISystem<Position>
	{
		bool& created;
		bool& destroyed;
		Tracker(bool& c, bool& d) : created{c}, destroyed{d} {}
		void OnCreate(ecs::World&) override
		{
			created = true;
		}
		void OnDestroy(ecs::World&) override
		{
			destroyed = true;
		}
		void Update(ecs::View<Position>) override {}
	};

	world.Register<Tracker>(ecs::Stage::Update, created, destroyed);
	EXPECT(created);
	EXPECT(!destroyed);

	world.Unregister<Tracker>();
	EXPECT(destroyed);
	PASS()
}

void test_system_multiframe()
{
	TEST(system_multiframe_accumulation)
	ecs::World world;

	float dt = 1.f;
	world.Register<MovementSystem>(ecs::Stage::Update, dt);

	auto e = world.CreateEntity();
	e.AddComponent<Position>(0.f, 0.f).AddComponent<Velocity>(1.f, 0.f);

	for (int i = 0; i < 10; ++i)
		world.RunStage(ecs::Stage::Update);

	EXPECT(e.GetComponent<Position>().x == 10.f);
	PASS()
}

// ──────────────────────────────────────────
// Main
// ──────────────────────────────────────────

int main()
{
	std::printf("╔══════════════════════════════════════╗\n");
	std::printf("║           ECS Test Suite             ║\n");
	std::printf("╚══════════════════════════════════════╝\n\n");

	std::printf("[ Entity ]\n");
	test_create_entity();
	test_destroy_entity();
	test_stale_handle();
	test_entity_count();

	std::printf("\n[ Components ]\n");
	test_add_get_component();
	test_has_component();
	test_try_get_component();
	test_remove_component();
	test_multiple_components();
	test_archetype_migration();
	test_swap_remove_correctness();
	test_component_add_remove_cycle();
	test_get_component_throws();

	std::printf("\n[ Entity API ]\n");
	test_entity_add_get_component();
	test_entity_chaining();
	test_entity_remove_component();
	test_entity_try_get_component();
	test_entity_destroy();
	test_entity_get_world();
	test_entity_handle_conversion();

	std::printf("\n[ Query / View ]\n");
	test_query_no_entity();
	test_query_with_entity();
	test_query_superset_archetypes();
	test_view_refresh();
	test_query_empty_world();
	test_query_no_matching_components();

	std::printf("\n[ Stress ]\n");
	test_large_entity_count();
	test_mass_destroy();

	std::printf("\n[ Systems ]\n");
	test_system_basic();
	test_system_delta_time();
	test_system_stage_ordering();
	test_system_priority();
	test_system_entity_destroy();
	test_system_oncreate_ondestroy();
	test_system_multiframe();

	std::printf("\n✓ All tests passed.\n");
}
