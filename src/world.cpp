#include <ecs/archetype.hpp>
#include <ecs/column.hpp>
#include <ecs/entity.hpp>
#include <ecs/entity_handle.hpp>
#include <ecs/signature.hpp>
#include <ecs/world.hpp>
#include <memory>
#include <stdexcept>

namespace ecs
{

World::World() : m_empty_archetype{GetOrCreateArchetype({})} {}

Entity World::CreateEntity()
{
	auto index = std::uint32_t{};

	if (not m_free_indices.empty())
	{
		index = m_free_indices.back();
		m_free_indices.pop_back();
	}
	else
	{
		index = static_cast<std::uint32_t>(m_records.size());
		m_records.emplace_back();
		m_generations.push_back(0);
	}

	auto  new_handle = EntityHandle{.index = index, .generation = m_generations[index]};
	auto& record	 = m_records[index];
	record.archetype = m_empty_archetype;
	record.row		 = m_empty_archetype->AllocateRowDefaultConstructed(new_handle);
	record.alive	 = true;

	return {*this, new_handle};
}

void World::DestroyEntity(EntityHandle handle)
{
	if (not IsAlive(handle))
		return;

	if (m_iterating)
	{
		m_pending_destroy.push_back(handle);
		return;
	}

	auto& record = m_records[handle.index];
	auto  result = record.archetype->RemoveRowSwap(record.row);

	if (result.moved)
	{
		m_records[result.moved_entity.index].row = record.row;
	}

	record.alive	 = false;
	record.archetype = nullptr;
	++m_generations[handle.index];
	m_free_indices.push_back(handle.index);
}

bool World::IsAlive(EntityHandle handle)
{
	// clang-format off
    return  (handle.index < m_records.size())
        and (m_records[handle.index].alive)
        and (m_generations[handle.index] == handle.generation);
	// clang-format on
}

std::size_t World::GetEntityCount() const noexcept
{
	return m_records.size() - m_free_indices.size();
}

void World::RunStage(Stage stage)
{
	m_iterating = true;
	for (auto& entry : m_systems)
	{
		if (entry.stage == stage)
		{
			entry.system->Run(*this);
		}
	}
	m_iterating = false;

	for (auto h : m_pending_destroy)
	{
		DestroyEntity(h);
	}
	m_pending_destroy.clear();
}

void World::RunAll()
{
	m_iterating = true;
	for (auto& entry : m_systems)
	{
		entry.system->Run(*this);
	}
	m_iterating = false;

	for (auto h : m_pending_destroy)
	{
		DestroyEntity(h);
	}
	m_pending_destroy.clear();
}

std::size_t World::GetRow(EntityHandle handle) const
{
	return m_records[handle.index].row;
}

std::uint64_t World::GetArchetypeVersion() const noexcept
{
	return m_archetype_version;
}

Entity World::HandleToEntity(EntityHandle handle)
{
	return {*this, handle};
}

ArcheType* World::GetOrCreateArchetype(ComponentSignature const& sig)
{
	auto it = m_archetypes.find(sig);

	if (it != m_archetypes.end())
	{
		return it->second.get();
	}

	auto  arch = std::make_unique<ArcheType>(sig);
	auto* ptr  = arch.get();

	m_archetypes.emplace(sig, std::move(arch));
	++m_archetype_version;

	return ptr;
}

ArcheType* World::ArchetypeAfterAdd(ArcheType* from, ComponentID ID)
{
	if (auto it = from->m_add_edges.find(ID); it != from->m_add_edges.end())
	{
		return it->second;
	}

	ComponentSignature sig = from->GetSignature();
	sig.Insert(ID);

	auto* to_archetype = GetOrCreateArchetype(sig);

	from->m_add_edges[ID]			 = to_archetype;
	to_archetype->m_remove_edges[ID] = from;

	return to_archetype;
}

ArcheType* World::ArchetypeAfterRemove(ArcheType* from, ComponentID ID)
{
	if (auto it = from->m_remove_edges.find(ID); it != from->m_remove_edges.end())
	{
		return it->second;
	}

	ComponentSignature sig = from->GetSignature();
	sig.Erase(ID);
	auto* to_archetype = GetOrCreateArchetype(sig);

	from->m_remove_edges[ID]	  = to_archetype;
	to_archetype->m_add_edges[ID] = from;

	return to_archetype;
}

std::size_t World::MoveBetweenArchetypes(EntityHandle e, ArcheType* old_arch, std::size_t old_row, ArcheType* new_arch)
{
	if (old_arch == new_arch)
	{
		throw std::runtime_error{"MoveBetweenArchetypes() transition must change archetype"};
	}

	auto new_row = new_arch->AllocateRowDefaultConstructed(e);

	for (auto c_ID : old_arch->GetSignature())
	{
		Column* dst = new_arch->GetColumn(c_ID);
		if (not dst)
		{
			continue; // Component dropped in this transition
		}

		Column* src = old_arch->GetColumn(c_ID);
		dst->MoveAssignFrom(new_row, *src, old_row);
	}

	auto result = old_arch->RemoveRowSwap(old_row);

	if (result.moved)
	{
		m_records[result.moved_entity.index].row = old_row;
	}

	return new_row;
}

std::vector<ArcheType*> World::ArchetypesMatching(ComponentSignature const& required) const
{
	auto out = std::vector<ArcheType*>{};

	for (auto& [sig, arch] : m_archetypes)
	{
		if (sig.ContainsAll(required))
		{
			out.push_back(arch.get());
		}
	}

	return out;
}

} // namespace ecs
