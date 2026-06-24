#pragma once

#include "entity_handle.hpp"
#include "fwd.hpp"

namespace ecs
{

struct EntityHandle;

class Entity
{
  public:
	template <typename Component, typename... Args> Entity& AddComponent(Args&&... args);
	template <typename Component> Entity&					RemoveComponent();
	template <typename Component> Component&				GetComponent();
	template <typename Component> Component*				TryGetComponent();
	template <typename Component> bool						HasComponent();

	void		 Destroy();
	bool		 IsAlive();
	EntityHandle GetHandle();
	World&		 GetWorld();

  public:
				operator EntityHandle() const noexcept;
	friend bool operator==(Entity const& lhs, Entity const& rhs) noexcept;

  private:
	Entity();
	Entity(World& owning_world, EntityHandle handle);
	friend World;

  private:
	World*		 m_owning_world = nullptr;
	EntityHandle m_handle		= EntityHandle::Invalid();
};

} // namespace ecs
