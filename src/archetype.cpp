#include <ecs/archetype.hpp>
#include <ecs/type_id.hpp>

namespace ecs
{

ArcheType::ArcheType(ComponentSignature signature) : m_signature{std::move(signature)}
{
	m_columns.reserve(m_signature.size());

	for (auto ID : m_signature)
	{
		m_columns.emplace_back(GetComponentInfo(ID));
	}
}

ComponentSignature const& ArcheType::GetSignature() const noexcept
{
	return m_signature;
}

std::size_t ArcheType::GetCount() const noexcept
{
	return m_entities.size();
}

EntityHandle ArcheType::GetEntityAt(std::size_t row) const
{
	return m_entities[row];
}

std::vector<EntityHandle> const& ArcheType::GetEntities() const noexcept
{
	return m_entities;
}

Column* ArcheType::GetColumn(ComponentID ID)
{
	auto index = TryGetColumnIndex(ID);
	return index.has_value() ? &(m_columns[index.value()]) : nullptr;
}

std::size_t ArcheType::AllocateRowDefaultConstructed(EntityHandle e)
{
	m_entities.push_back(e);
	for (auto& column : m_columns)
	{
		column.PushDefaultConstructed();
	}
	return m_entities.size() - 1;
}

ArcheTyperemoveResult ArcheType::RemoveRowSwap(std::size_t row)
{
	if (m_entities.size() == 0 or row >= m_entities.size())
	{
		throw std::runtime_error{"Calling RemoveRowSwap() on an empty archetype."};
	}

	auto result = ArcheTyperemoveResult{};
	auto last	= m_entities.size() - 1;

	if (row != last)
	{
		m_entities[row]		= m_entities[last];
		result.moved		= true;
		result.moved_entity = m_entities[row];
	}

	m_entities.pop_back();

	for (auto& col : m_columns)
	{
		col.SwapRemove(row);
	}

	return result;
}

std::optional<std::size_t> ArcheType::TryGetColumnIndex(ComponentID ID) const
{
	auto i = 0zu;
	for (auto c_ID : m_signature)
	{
		if (c_ID == ID)
		{
			return i;
		}

		++i;
	}

	return {};
}

} // namespace ecs
