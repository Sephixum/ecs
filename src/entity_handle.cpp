#include <ecs/entity_handle.hpp>

std::size_t std::hash<ecs::EntityHandle>::operator()(ecs::EntityHandle const& e) noexcept
{
	return (static_cast<std::size_t>(e.index) << 32) | e.generation;
}
