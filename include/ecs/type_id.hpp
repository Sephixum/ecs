#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace ecs
{

using ComponentID								  = std::uint32_t;
inline constexpr ComponentID INVALID_COMPONENT_ID = static_cast<ComponentID>(-1);

struct ComponentInfo
{
	std::size_t size  = {};
	std::size_t align = {};

	void (*default_construct)(void* dst)		 = nullptr;
	void (*move_construct)(void* dst, void* src) = nullptr;
	void (*move_assign)(void* dst, void* src)	 = nullptr;
	void (*destroy)(void* dst)					 = nullptr;
};

namespace detail
{

// ComponentID Generator
constexpr ComponentID GetNextComponentID() noexcept
{
	static auto counter = ComponentID{};
	return counter++;
}

// Global table from ComponentID to ComponentInfo *
// Lets generic archtype/column code look up operations
// by runtime ID.
constexpr std::vector<ComponentInfo const*>& GetComponentInfoTable()
{
	static auto table = std::vector<ComponentInfo const*>{};
	return table;
}

// Register ComponentInfo and change to resize the info
// in the table
constexpr void RegisterComponentInfo(ComponentID const id, ComponentInfo const* info)
{
	auto& table = GetComponentInfoTable();
	if (table.size() <= id)
	{
		table.resize(id + 1, nullptr);
	}
	table[id] = info;
}

} // namespace detail

template <typename T> constexpr ComponentInfo const& GetComponentInfo()
{
	static_assert(std::default_initializable<T>, "Components must be default constructible");
	static_assert(std::is_nothrow_move_constructible_v<T>, "Components must be no throw move constructible");
	// clang-format off
	static auto const info = ComponentInfo
    {
		.size			   = sizeof(T),
		.align			   = alignof(T),
		.default_construct = [](void* src) { std::construct_at(static_cast<T*>(src)); },
		.move_construct	   = [](void* dst, void* src) { std::construct_at(static_cast<T*>(dst), std::move(*static_cast<T*>(src))); },
		.move_assign       = [](void* dst, void* src) { *static_cast<T*>(dst) = std::move(*static_cast<T*>(src)); },
		.destroy	       = [](void* obj) { std::destroy_at(static_cast<T*>(obj)); }

	};
	// clang-format on
	return info;
}

constexpr ComponentInfo const& GetComponentInfo(ComponentID ID)
{
	return *(detail::GetComponentInfoTable()[ID]);
}

template <typename T> ComponentID GetComponentID()
{
	static auto const ID = [] -> ComponentID
	{
		auto const new_ID = detail::GetNextComponentID();
		detail::RegisterComponentInfo(new_ID, &GetComponentInfo<T>());
		return new_ID;
	}();
	return ID;
}

} // namespace ecs
