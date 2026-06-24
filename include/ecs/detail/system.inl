#pragma once

#include <ecs/world.hpp>

namespace ecs
{

template <typename... Components> void ISystem<Components...>::Run(World& world)
{
	Update(world.template Query<Components...>());
}

} // namespace ecs
