#pragma once

#include "column.hpp"
#include "entity_handle.hpp"
#include "signature.hpp"
#include "type_id.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

namespace ecs
{

struct ArcheTyperemoveResult
{
	bool		 moved		  = false;
	EntityHandle moved_entity = {};
};

class ArcheType
{
  public:
	explicit ArcheType(ComponentSignature signature);

	ComponentSignature const&		 GetSignature() const noexcept;
	std::size_t						 GetCount() const noexcept;
	EntityHandle					 GetEntityAt(std::size_t row) const;
	Column*							 GetColumn(ComponentID ID);
	std::size_t						 AllocateRowDefaultConstructed(EntityHandle e);
	ArcheTyperemoveResult			 RemoveRowSwap(std::size_t row);
	std::vector<EntityHandle> const& GetEntities() const noexcept;

  private:
	friend class World;
	std::unordered_map<ComponentID, ArcheType*> m_add_edges;
	std::unordered_map<ComponentID, ArcheType*> m_remove_edges;

  private:
	std::optional<std::size_t> TryGetColumnIndex(ComponentID ID) const;

  private:
	ComponentSignature		  m_signature;
	std::vector<Column>		  m_columns;
	std::vector<EntityHandle> m_entities;
};

} // namespace ecs
