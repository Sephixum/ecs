#include <ecs/entity.hpp>
#include <ecs/world.hpp>

namespace ecs
{

Entity::Entity(World& owning_world, EntityHandle handle) : m_owning_world{&owning_world}, m_handle{handle} {}

void Entity::Destroy()
{
	m_owning_world->DestroyEntity(m_handle);
	m_handle = EntityHandle::Invalid();
}

bool Entity::IsAlive()
{
	return m_owning_world->IsAlive(m_handle);
}

EntityHandle Entity::GetHandle()
{
	return m_handle;
}

World& Entity::GetWorld()
{
	return *m_owning_world;
}

Entity::operator EntityHandle() const noexcept
{
	return m_handle;
}

bool operator==(Entity const& lhs, Entity const& rhs) noexcept
{
	return lhs.m_handle == rhs.m_handle && lhs.m_owning_world == rhs.m_owning_world;
}

} // namespace ecs
