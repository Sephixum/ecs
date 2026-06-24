#pragma once

#include <ecs/archetype.hpp>
#include <ecs/view.hpp>
#include <ecs/world.hpp>
#include <memory>
#include <optional>
#include <stdexcept>

namespace ecs
{

template <typename Component, typename... Args> Component& World::AddComponent(EntityHandle e, Args&&... args)
{
	auto  id	 = GetComponentID<Component>();
	auto& record = m_records[e.index];

	ArcheType*	new_arch = ArchetypeAfterAdd(record.archetype, id);
	std::size_t new_row	 = MoveBetweenArchetypes(e, record.archetype, record.row, new_arch);

	record.archetype = new_arch;
	record.row		 = new_row;

	Component* ptr = new_arch->GetColumn(id)->template ViewAs<Component>() + new_row;
	std::destroy_at(ptr);
	std::construct_at(ptr, std::forward<Args>(args)...);

	return *ptr;
}

template <typename Component> void World::RemoveComponent(EntityHandle e)
{
	auto  id	 = GetComponentID<Component>();
	auto& record = m_records[e.index];

	ArcheType*	new_arch = ArchetypeAfterRemove(record.archetype, id);
	std::size_t new_row	 = MoveBetweenArchetypes(e, record.archetype, record.row, new_arch);

	record.archetype = new_arch;
	record.row		 = new_row;
}

template <typename Component> Component* World::TryGetComponent(EntityHandle e)
{
	auto& record = m_records[e.index];
	auto* col	 = record.archetype->GetColumn(GetComponentID<Component>());

	if (not col)
	{
		return nullptr;
	}

	return col->template ViewAs<Component>() + record.row;
}

template <typename Component> Component& World::GetComponent(EntityHandle e)
{
	auto* c_ptr = TryGetComponent<Component>(e);

	if (not c_ptr)
	{
		throw std::runtime_error{"World::GetComponent(): Component does not exist."};
	}

	return *c_ptr;
}

template <typename Component> bool World::HasComponent(EntityHandle e)
{
	auto& record = m_records[e.index];
	return record.archetype->GetColumn(GetComponentID<Component>()) != nullptr;
}

template <typename... Components> View<Components...> World::Query()
{
	auto required = ComponentSignature::Create<Components...>();
	return View<Components...>{*this, ArchetypesMatching(required), m_archetype_version};
}

template <typename S, typename... Args> S& World::Register(Stage stage, Args&&... args)
{
	return Register<S>(stage, DEFAULT_SYSTEM_PRIORITY, std::forward<Args>(args)...);
}

template <typename S, typename... Args> S& World::Register(Stage stage, Priority priority, Args&&... args)
{
	static_assert(std::is_base_of_v<BaseSystem, S>, "S must derive from BaseSystem");

	auto  entry = SystemEntry{std::make_unique<S>(std::forward<Args>(args)...), stage, priority};
	auto* raw	= static_cast<S*>(entry.system.get());
	auto  it	= std::ranges::upper_bound(m_systems,
										   entry,
										   [](SystemEntry const& a, SystemEntry const& b)
										   {
										   if (a.stage != b.stage)
										   {
											   return static_cast<std::uint32_t>(a.stage) <
													  static_cast<std::uint32_t>(b.stage);
										   }
										   return a.priority < b.priority;
										   });

	m_systems.insert(it, std::move(entry));
	raw->OnCreate(*this);
	return *raw;
}

template <typename S> void World::Unregister()
{
	auto it = std::ranges::find_if(m_systems,
								   [](SystemEntry const& e) { return dynamic_cast<S*>(e.system.get()) != nullptr; });

	if (it == m_systems.end())
	{
		return;
	}

	it->system->OnDestroy(*this);
	m_systems.erase(it);
}

} // namespace ecs
