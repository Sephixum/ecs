#pragma once

#include <ecs/entity.hpp>
#include <ecs/world.hpp>

namespace ecs
{

template <typename Component, typename... Args> Entity& Entity::AddComponent(Args&&... args)
{
	m_owning_world->AddComponent<Component>(m_handle, std::forward<Args>(args)...);
	return *this;
}

template <typename Component> Entity& Entity::RemoveComponent()
{
	m_owning_world->RemoveComponent<Component>(m_handle);
	return *this;
}

template <typename Component> Component& Entity::GetComponent()
{
	return m_owning_world->GetComponent<Component>(m_handle);
}

template <typename Component> Component* Entity::TryGetComponent()
{
	return m_owning_world->TryGetComponent<Component>(m_handle);
}

template <typename Component> bool Entity::HasComponent()
{
	return m_owning_world->HasComponent<Component>(m_handle);
}

} // namespace ecs
