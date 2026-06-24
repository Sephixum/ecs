#pragma once

#include <ecs/view.hpp>
#include <ecs/world.hpp>
#include <ranges>

namespace ecs
{

namespace vws = std::views;

template <typename... Components>
View<Components...>::View(World& world, std::vector<ArcheType*> matching, std::uint64_t version)
	: m_world{&world}, m_matching{std::move(matching)}, m_world_version{version}
{
}

template <typename... Components> void View<Components...>::RefreshIfStale()
{
	if (m_world_version == m_world->GetArchetypeVersion())
	{
		return;
	}
	auto required	= ComponentSignature::Create<Components...>();
	m_matching		= m_world->ArchetypesMatching(required);
	m_world_version = m_world->GetArchetypeVersion();
}

template <typename... Components>
template <typename Func>
	requires std::invocable<Func, Components&...>
void View<Components...>::Each(Func&& func)
{
	RefreshIfStale();
	for (ArcheType* arch : m_matching)
	{
		std::tuple<Components*...> cols{
			arch->GetColumn(GetComponentID<Components>())->template ViewAs<Components>()...};

		for (auto row : vws::iota(0zu, arch->GetCount()))
		{
			std::apply([&](Components*... base) { func(base[row]...); }, cols);
		}
	}
}

template <typename... Components>
template <typename Func>
	requires std::invocable<Func, Entity, Components&...>
void View<Components...>::Each(Func&& func)
{
	RefreshIfStale();
	for (ArcheType* arch : m_matching)
	{
		std::tuple<Components*...> cols{
			arch->GetColumn(GetComponentID<Components>())->template ViewAs<Components>()...};

		for (auto row : vws::iota(0zu, arch->GetCount()))
		{
			std::apply([&](Components*... base)
					   { func(m_world->HandleToEntity(arch->GetEntityAt(row)), base[row]...); },
					   cols);
		}
	}
}

} // namespace ecs
