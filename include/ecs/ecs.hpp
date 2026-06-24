#pragma once

// primitives — no dependencies
#include "entity.hpp"
#include "entity_handle.hpp"
#include "signature.hpp"
#include "type_id.hpp"

// column — depends on type_id
#include "column.hpp"

// archetype — depends on column, signature, entity_handle
#include "archetype.hpp"

// world — depends on archetype
#include "world.hpp"

// view — depends on world, archetype
#include "view.hpp"

#include "detail/entity.inl"
#include "detail/system.inl"
#include "detail/view.inl"
#include "detail/world.inl"
