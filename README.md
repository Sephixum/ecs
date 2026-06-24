# ecs

A fast, archetype-based Entity Component System written in C++23.

Iteration performance is the primary design goal. Components are stored in contiguous archetype blocks — querying a set of components is a linear scan over packed memory with no indirection.

```cpp
ecs::World world;

world.Register<MovementSystem>(ecs::Stage::Update);

for (int i = 0; i < 10'000; ++i)
    world.CreateEntity()
        .AddComponent<Position>((float)i, 0.f)
        .AddComponent<Velocity>(1.f, 0.f);

world.RunStage(ecs::Stage::Update);
```

---

## Features

- Archetype storage — components grouped by signature into contiguous arrays
- Zero-overhead queries — `View` iterates packed memory, no pointer chasing
- Automatic view invalidation — views re-resolve archetypes when the world changes
- Staged systems — `PreUpdate`, `Update`, `PostUpdate` with explicit priority ordering
- Safe structural changes during iteration — deferred entity destruction
- Generation-checked entity handles — dead entity access is detected
- C++23, no dependencies, single umbrella header

---

## Building

Requires CMake 3.25+ and a C++23-capable compiler (GCC 13+, Clang 16+).

```bash
git clone https://github.com/you/ecs
cd ecs
cmake -B build/debug  -DCMAKE_BUILD_TYPE=Debug
cmake -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/debug
```

Run tests:
```bash
ctest --test-dir build/debug --output-on-failure
```

Run benchmarks:
```bash
cmake --build build/release --target bench
./build/release/bench/bench
```

### Options

| Option | Default | Description |
|---|---|---|
| `ECS_BUILD_TESTS` | `ON` | Build the test suite |
| `ECS_BUILD_BENCH` | `ON` | Build the benchmark binary |

---

## Quick Start

Include the umbrella header:

```cpp
#include <ecs/ecs.hpp>
```

### Components

Plain structs, no registration required:

```cpp
struct Position { float x, y; };
struct Velocity { float x, y; };
struct Health   { float hp;   };
```

### World & Entities

```cpp
ecs::World world;

// create
ecs::Entity e = world.CreateEntity();
e.AddComponent<Position>(0.f, 0.f);
e.AddComponent<Velocity>(1.f, 0.f);

// fluent chaining
world.CreateEntity()
    .AddComponent<Position>(0.f, 0.f)
    .AddComponent<Velocity>(1.f, 0.f)
    .AddComponent<Health>(100.f);

// access
Position& p = e.GetComponent<Position>();
Health*   h = e.TryGetComponent<Health>(); // nullptr if absent

// remove / destroy
e.RemoveComponent<Velocity>();
e.Destroy();

// query world state
world.GetEntityCount();
world.IsAlive(handle);
```

### Querying with Views

`Query` returns a typed `View` that iterates all archetypes containing the requested components.

```cpp
auto view = world.Query<Position, Velocity>();

// without entity
view.Each([](Position& p, Velocity& v) {
    p.x += v.x;
    p.y += v.y;
});

// with entity
view.Each([](ecs::Entity e, Position& p, Velocity& v) {
    if (p.x > 1000.f)
        e.Destroy();
});
```

Views cache their matching archetypes and automatically re-resolve when the world's archetype layout changes.

### Systems

Derive from `ISystem<Components...>` and implement `Update`:

```cpp
class MovementSystem : public ecs::ISystem<Position, Velocity>
{
public:
    void Update(ecs::View<Position, Velocity> view) override
    {
        view.Each([](Position& p, Velocity& v) {
            p.x += v.x;
            p.y += v.y;
        });
    }
};
```

Optional lifecycle hooks:

```cpp
class MySystem : public ecs::ISystem<Position>
{
public:
    void OnCreate(ecs::World& world) override  { /* called once on Register */ }
    void OnDestroy(ecs::World& world) override { /* called once on Unregister */ }
    void Update(ecs::View<Position> view) override { /* called each RunStage */ }
};
```

### Registering Systems

```cpp
// default priority (0)
world.Register<MovementSystem>(ecs::Stage::Update);

// explicit priority — lower value runs first
world.Register<PhysicsSystem>(ecs::Stage::Update, ecs::Priority{-10});
world.Register<RenderSystem> (ecs::Stage::PostUpdate);

// pass constructor arguments
world.Register<NetworkSystem>(ecs::Stage::Update, host, port);

// unregister
world.Unregister<MovementSystem>();
```

### Stages

Systems are bucketed into three ordered stages:

| Stage | Value | Typical use |
|---|---|---|
| `ecs::Stage::PreUpdate` | 0 | Input, timers |
| `ecs::Stage::Update` | 1 | Game logic, physics |
| `ecs::Stage::PostUpdate` | 2 | Rendering, networking |

```cpp
world.RunStage(ecs::Stage::Update); // run one stage
world.RunAll();                     // run all three in order
```

Within a stage, systems execute in ascending priority order. Systems with equal priority run in registration order.

---

## Design Notes

**Why archetypes?**
Each unique combination of components maps to one archetype. Archetypes store their components in separate contiguous arrays (SoA). Querying `<Position, Velocity>` walks only the archetypes that have both — no sparse lookups, no per-entity branching.

**Tradeoff**
Adding or removing a component moves an entity between archetypes, which involves a copy. This ECS optimises for systems that iterate millions of entities per frame, not for worlds where structural changes dominate.

**Structural safety**
Destroying an entity inside `Each` is safe. Destruction is deferred until the current stage finishes, so iterators are never invalidated mid-loop.

---

## Performance

Benchmarked on a single archetype of 1 000 000 entities, 100 frames, `Position` + `Velocity` update:

| | Time | ns / entity |
|---|---|---|
| ecs (system) | 5.1 ms | 0.51 ns |

Hardware: [AMD ryzen5 5500u]. Compiled with `-O3 -march=native`.

---

## Project Structure

```
ecs/
├── include/ecs/        # public headers + detail/*.inl
├── src/                # translation units
└── tests/              # test suite
```

---

## License

MIT
