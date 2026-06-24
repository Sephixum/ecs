#pragma once

#include <cstdint>
#include <unordered_map>

namespace ecs
{

struct EntityHandle
{
	std::uint32_t index		 = static_cast<std::uint32_t>(-1);
	std::uint32_t generation = 0;

	static constexpr EntityHandle Invalid() noexcept
	{
		return {};
	}

	constexpr explicit operator bool() const noexcept
	{
		return (index != static_cast<std::uint32_t>(-1));
	}

	friend constexpr bool operator==(EntityHandle const&, EntityHandle const&) = default;
};

} // namespace ecs

template <> struct std::hash<ecs::EntityHandle>
{
	static std::size_t operator()(ecs::EntityHandle const& e) noexcept;
};
