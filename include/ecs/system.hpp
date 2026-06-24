#pragma once

#include "fwd.hpp"
#include "view.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

namespace ecs
{

enum struct Stage : std::uint32_t
{
	PreUpdate  = 0,
	Update	   = 1,
	PostUpdate = 2,
};

struct Priority
{
	std::int32_t value = 0;

	constexpr explicit Priority(std::int32_t v) noexcept : value{v} {}

	constexpr auto operator<=>(Priority const& b) const noexcept = default;

	constexpr operator std::int32_t() const noexcept
	{
		return value;
	}
};

inline constexpr auto DEFAULT_SYSTEM_PRIORITY = Priority{0};

struct BaseSystem
{
	virtual void Run(World& world) = 0;
	virtual ~BaseSystem()		   = default;

	virtual void OnCreate(World&) {}
	virtual void OnDestroy(World&) {}
};

template <typename... Components> struct ISystem : BaseSystem
{
	virtual void Update(View<Components...> view) = 0;

	void Run(World& world) final;
};

} // namespace ecs
