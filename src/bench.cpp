#include <chrono>
#include <cstdio>
#include <ecs/ecs.hpp>
#include <entt/entt.hpp>
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

// ──────────────────────────────────────────
// Timer
// ──────────────────────────────────────────

struct Timer
{
	using Clock = std::chrono::high_resolution_clock;
	void Start()
	{
		m_start = Clock::now();
	}
	double ElapsedMs() const
	{
		return std::chrono::duration<double, std::milli>(Clock::now() - m_start).count();
	}
	Clock::time_point m_start;
};

// ──────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────

static void print_header(char const* label)
{
	std::printf("\n┌─────────────────────────────────────────┐\n");
	std::printf("│  %-40s│\n", label);
	std::printf("└─────────────────────────────────────────┘\n");
}

static void print_row(char const* name, double ms, int N)
{
	double per_entity_ns = (ms * 1e6) / N;
	std::printf("  %-35s %8.3f ms  (%6.2f ns/entity)\n", name, ms, per_entity_ns);
}

// ──────────────────────────────────────────
// Benchmarks
// ──────────────────────────────────────────

void bench_create(int N)
{
	print_header("Create entities");
	Timer t;

	{
		ecs::World world;
		t.Start();
		for (int i = 0; i < N; ++i)
		{
			world.CreateEntity();
		}
		print_row("custom ECS", t.ElapsedMs(), N);
	}

	{
		entt::registry registry;
		t.Start();
		for (int i = 0; i < N; ++i)
		{
			auto _ = registry.create();
		}
		print_row("EnTT", t.ElapsedMs(), N);
	}
}

void bench_create_add(int N)
{
	print_header("Create + AddComponent (Position, Velocity)");
	Timer t;

	{
		ecs::World world;
		t.Start();
		for (int i = 0; i < N; ++i)
		{
			world.CreateEntity().AddComponent<Position>((float)i, 0.f).AddComponent<Velocity>(1.f, 0.f);
		}
		print_row("custom ECS", t.ElapsedMs(), N);
	}

	{
		entt::registry registry;
		t.Start();
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e, (float)i, 0.f);
			registry.emplace<Velocity>(e, 1.f, 0.f);
		}
		print_row("EnTT", t.ElapsedMs(), N);
	}
}

void bench_iterate_two(int N)
{
	print_header("Iterate (Position, Velocity)");
	Timer t;

	{
		ecs::World world;
		for (int i = 0; i < N; ++i)
		{
			world.CreateEntity().AddComponent<Position>((float)i, 0.f).AddComponent<Velocity>(1.f, 0.f);
		}
		auto view = world.Query<Position, Velocity>();

		float volatile sink = 0.f;
		t.Start();
		view.Each(
			[&sink](Position& p, Velocity& v)
			{
				p.x += v.x;
				p.y += v.y;
				sink = p.x;
			});
		print_row("custom ECS", t.ElapsedMs(), N);
	}

	{
		entt::registry registry;
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e, (float)i, 0.f);
			registry.emplace<Velocity>(e, 1.f, 0.f);
		}
		auto view = registry.view<Position, Velocity>();

		float volatile sink = 0.f;
		t.Start();
		view.each(
			[&sink](Position& p, Velocity& v)
			{
				p.x += v.x;
				p.y += v.y;
				sink = p.x;
			});
		print_row("EnTT", t.ElapsedMs(), N);
	}
}

void bench_iterate_three(int N)
{
	print_header("Iterate (Position, Velocity, Health)");
	Timer t;

	{
		ecs::World world;
		for (int i = 0; i < N; ++i)
		{
			world.CreateEntity()
				.AddComponent<Position>((float)i, 0.f)
				.AddComponent<Velocity>(1.f, 0.f)
				.AddComponent<Health>(100.f);
		}
		auto view = world.Query<Position, Velocity, Health>();

		float volatile sink = 0.f;
		t.Start();
		view.Each(
			[&sink](Position& p, Velocity& v, Health& h)
			{
				p.x += v.x;
				h.value -= 1.f;
				sink = p.x;
			});
		print_row("custom ECS", t.ElapsedMs(), N);
	}

	{
		entt::registry registry;
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e, (float)i, 0.f);
			registry.emplace<Velocity>(e, 1.f, 0.f);
			registry.emplace<Health>(e, 100.f);
		}
		auto view = registry.view<Position, Velocity, Health>();

		float volatile sink = 0.f;
		t.Start();
		view.each(
			[&sink](Position& p, Velocity& v, Health& h)
			{
				p.x += v.x;
				h.value -= 1.f;
				sink = p.x;
			});
		print_row("EnTT", t.ElapsedMs(), N);
	}
}

void bench_mixed_archetypes(int N)
{
	print_header("Iterate mixed archetypes (Position, Velocity)");
	Timer t;

	{
		ecs::World world;
		for (int i = 0; i < N; ++i)
		{
			auto e = world.CreateEntity().AddComponent<Position>((float)i, 0.f).AddComponent<Velocity>(1.f, 0.f);
			if (i % 2 == 0)
				e.AddComponent<Health>(100.f);
		}
		auto view = world.Query<Position, Velocity>();

		float volatile sink = 0.f;
		t.Start();
		view.Each(
			[&sink](Position& p, Velocity& v)
			{
				p.x += v.x;
				sink = p.x;
			});
		print_row("custom ECS", t.ElapsedMs(), N);
	}

	{
		entt::registry registry;
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e, (float)i, 0.f);
			registry.emplace<Velocity>(e, 1.f, 0.f);
			if (i % 2 == 0)
				registry.emplace<Health>(e, 100.f);
		}
		auto view = registry.view<Position, Velocity>();

		float volatile sink = 0.f;
		t.Start();
		view.each(
			[&sink](Position& p, Velocity& v)
			{
				p.x += v.x;
				sink = p.x;
			});
		print_row("EnTT", t.ElapsedMs(), N);
	}
}

void bench_destroy(int N)
{
	print_header("Destroy all entities");
	Timer t;

	{
		ecs::World				 world;
		std::vector<ecs::Entity> entities;
		entities.reserve(N);
		for (int i = 0; i < N; ++i)
		{
			auto e = world.CreateEntity();
			e.AddComponent<Position>();
			e.AddComponent<Velocity>();
			entities.push_back(e);
		}

		t.Start();
		for (auto e : entities)
		{
			e.Destroy();
		}
		print_row("custom ECS", t.ElapsedMs(), N);
	}

	{
		entt::registry			  registry;
		std::vector<entt::entity> handles;
		handles.reserve(N);
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e);
			registry.emplace<Velocity>(e);
			handles.push_back(e);
		}

		t.Start();
		for (auto e : handles)
		{
			registry.destroy(e);
		}
		print_row("EnTT", t.ElapsedMs(), N);
	}
}

// ──────────────────────────────────────────
// Systems for benchmark
// ──────────────────────────────────────────

class MovementSystem : public ecs::ISystem<Position, Velocity>
{
  public:
	float volatile sink = 0.0f;

	void Update(ecs::View<Position, Velocity> view) override
	{
		view.Each(
			[&](Position& p, Velocity& v)
			{
				p.x += v.x;
				p.y += v.y;
				sink = p.x; // ← force the work to matter
			});
	}
};

class HealthDecaySystem : public ecs::ISystem<Position, Velocity, Health>
{
  public:
	float volatile sink = 0.0f;

	void Update(ecs::View<Position, Velocity, Health> view) override
	{
		view.Each(
			[&](Position& p, Velocity& v, Health& h)
			{
				p.x += v.x;
				h.value -= 1.f;
				sink = p.x; // ← force the work to matter
			});
	}
};

// ──────────────────────────────────────────
// System Benchmarks
// ──────────────────────────────────────────

void bench_system_two_components(int N)
{
	print_header("System update (Position, Velocity)");
	Timer t;
	{
		ecs::World world;
		world.Register<MovementSystem>(ecs::Stage::Update);
		for (int i = 0; i < N; ++i)
			world.CreateEntity().AddComponent<Position>((float)i, 0.f).AddComponent<Velocity>(1.f, 0.f);

		t.Start();
		world.RunStage(ecs::Stage::Update);
		print_row("custom ECS (system)", t.ElapsedMs(), N);
	}
	{
		// EnTT has no built-in system abstraction — raw view.each() is the equivalent
		entt::registry registry;
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e, (float)i, 0.f);
			registry.emplace<Velocity>(e, 1.f, 0.f);
		}
		auto view = registry.view<Position, Velocity>();
		t.Start();
		view.each(
			[](Position& p, Velocity& v)
			{
				p.x += v.x;
				p.y += v.y;
			});
		print_row("EnTT (raw view)", t.ElapsedMs(), N);
	}
}

void bench_system_three_components(int N)
{
	print_header("System update (Position, Velocity, Health)");
	Timer t;
	{
		ecs::World world;
		world.Register<HealthDecaySystem>(ecs::Stage::Update);
		for (int i = 0; i < N; ++i)
			world.CreateEntity()
				.AddComponent<Position>((float)i, 0.f)
				.AddComponent<Velocity>(1.f, 0.f)
				.AddComponent<Health>(100.f);

		t.Start();
		world.RunStage(ecs::Stage::Update);
		print_row("custom ECS (system)", t.ElapsedMs(), N);
	}
	{
		entt::registry registry;
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e, (float)i, 0.f);
			registry.emplace<Velocity>(e, 1.f, 0.f);
			registry.emplace<Health>(e, 100.f);
		}
		auto view = registry.view<Position, Velocity, Health>();
		t.Start();
		view.each(
			[](Position& p, Velocity& v, Health& h)
			{
				p.x += v.x;
				h.value -= 1.f;
			});
		print_row("EnTT (raw view)", t.ElapsedMs(), N);
	}
}

void bench_system_multiframe(int N, int frames)
{
	print_header("System update — 100 frames (Position, Velocity)");
	Timer t;
	{
		ecs::World world;
		world.Register<MovementSystem>(ecs::Stage::Update);
		for (int i = 0; i < N; ++i)
		{
			world.CreateEntity().AddComponent<Position>((float)i, 0.f).AddComponent<Velocity>(1.f, 0.f);
		}

		t.Start();
		for (int f = 0; f < frames; ++f)
		{
			world.RunStage(ecs::Stage::Update);
		}
		print_row("custom ECS (system)", t.ElapsedMs(), N * frames);
	}
	{
		entt::registry registry;
		for (int i = 0; i < N; ++i)
		{
			auto e = registry.create();
			registry.emplace<Position>(e, (float)i, 0.f);
			registry.emplace<Velocity>(e, 1.f, 0.f);
		}
		auto view = registry.view<Position, Velocity>();
		t.Start();
		for (int f = 0; f < frames; ++f)
			view.each(
				[](Position& p, Velocity& v)
				{
					p.x += v.x;
					p.y += v.y;
				});
		print_row("EnTT (raw view)", t.ElapsedMs(), N * frames);
	}
}

// ──────────────────────────────────────────
// Main
// ──────────────────────────────────────────

int main()
{
	std::printf("╔══════════════════════════════════════════╗\n");
	std::printf("║         ECS Benchmark vs EnTT            ║\n");
	std::printf("╚══════════════════════════════════════════╝\n");

	constexpr int N = 100000;
	std::printf("  N = %d entities per benchmark\n", N);

	bench_create(N);
	bench_create_add(N);
	bench_iterate_two(N);
	bench_iterate_three(N);
	bench_mixed_archetypes(N);
	bench_destroy(N);
	constexpr int frames = 100;
	bench_system_two_components(N);
	bench_system_three_components(N);
	bench_system_multiframe(N, frames);

	std::printf("\n");
}
