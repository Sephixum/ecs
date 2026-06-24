#pragma once

#include "archetype.hpp"
#include "entity_handle.hpp"
#include "fwd.hpp"
#include <vector>

namespace ecs
{

template <typename... Components> class View
{
  public:
	explicit View(World& world, std::vector<ArcheType*> matching, std::uint64_t version);

	template <typename Func>
		requires std::invocable<Func, Components&...>
	void Each(Func&& func);

	template <typename Func>
		requires std::invocable<Func, Entity, Components&...>
	void Each(Func&& func);

  private:
	void RefreshIfStale();

  private:
	World*					m_world			= nullptr;
	std::vector<ArcheType*> m_matching		= {};
	std::uint64_t			m_world_version = 0;
};

} // namespace ecs
