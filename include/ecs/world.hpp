#pragma once

#include "entity_handle.hpp"
#include "fwd.hpp"
#include "signature.hpp"
#include "system.hpp"
#include "type_id.hpp"
#include <memory>
#include <unordered_map>

namespace ecs
{

class World
{
  public:
	World();

  public:
	Entity		CreateEntity();
	void		DestroyEntity(EntityHandle e);
	bool		IsAlive(EntityHandle e);
	std::size_t GetEntityCount() const noexcept;

	template <typename... Components> View<Components...> Query();

	// systems
	template <typename S, typename... Args> S& Register(Stage stage, Args&&... args);
	template <typename S, typename... Args> S& Register(Stage stage, Priority priority, Args&&... args);
	template <typename S> void				   Unregister();

	void		RunStage(Stage stage);
	void		RunAll();
	std::size_t GetRow(EntityHandle handle) const;

  private:
	template <typename Component, typename... Args> Component& AddComponent(EntityHandle e, Args&&... args);
	template <typename Component> void						   RemoveComponent(EntityHandle e);
	template <typename Component> Component*				   TryGetComponent(EntityHandle e);
	template <typename Component> Component&				   GetComponent(EntityHandle e);
	template <typename Component> bool						   HasComponent(EntityHandle e);

  private:
	friend class Entity;
	template <typename... Components> friend class View;

	Entity HandleToEntity(EntityHandle handle);

	struct EntityRecord
	{
		ArcheType*	archetype = nullptr;
		std::size_t row		  = 0;
		bool		alive	  = false;
	};

	struct SystemEntry
	{
		std::unique_ptr<BaseSystem> system	 = nullptr;
		Stage						stage	 = Stage::Update;
		Priority					priority = DEFAULT_SYSTEM_PRIORITY;
	};

  private:
	std::uint64_t GetArchetypeVersion() const noexcept;
	ArcheType*	  GetOrCreateArchetype(ComponentSignature const& sig);
	ArcheType*	  ArchetypeAfterAdd(ArcheType* from, ComponentID id);
	ArcheType*	  ArchetypeAfterRemove(ArcheType* from, ComponentID id);
	std::size_t	  MoveBetweenArchetypes(EntityHandle e, ArcheType* old_arch, std::size_t old_row, ArcheType* new_arch);
	std::vector<ArcheType*> ArchetypesMatching(ComponentSignature const& required) const;

  private:
	std::vector<EntityRecord>										   m_records;
	std::vector<std::uint32_t>										   m_generations;
	std::vector<std::uint32_t>										   m_free_indices;
	std::unordered_map<ComponentSignature, std::unique_ptr<ArcheType>> m_archetypes;
	ArcheType*														   m_empty_archetype   = nullptr;
	std::uint64_t													   m_archetype_version = 0;
	std::vector<SystemEntry>										   m_systems;
	bool															   m_iterating		 = false;
	std::vector<EntityHandle>										   m_pending_destroy = {};
};

} // namespace ecs
