#include <algorithm>
#include <cstddef>
#include <ecs/signature.hpp>
#include <ecs/type_id.hpp>

namespace ecs
{

namespace rng = std::ranges;

void ComponentSignature::Insert(ComponentID ID)
{
	auto it = rng::lower_bound(m_IDs, ID);
	if (it == m_IDs.end() or (*it) != ID)
	{
		m_IDs.insert(it, ID);
	}
}

void ComponentSignature::Erase(ComponentID ID)
{
	auto it = rng::lower_bound(m_IDs, ID);
	if (it != m_IDs.end() and (*it) == ID)
	{
		m_IDs.erase(it);
	}
}

bool ComponentSignature::Contains(ComponentID ID)
{
	return rng::binary_search(m_IDs, ID);
}

bool ComponentSignature::ContainsAll(ComponentSignature const& other) const
{
	return rng::includes(m_IDs, other.m_IDs);
}

std::size_t ComponentSignature::size() const noexcept
{
	return m_IDs.size();
}

bool ComponentSignature::empty() const noexcept
{
	return m_IDs.empty();
}

auto ComponentSignature::begin() const noexcept -> const_iterator
{
	return m_IDs.begin();
}

auto ComponentSignature::end() const noexcept -> const_iterator
{
	return m_IDs.end();
}

} // namespace ecs

std::size_t std::hash<ecs::ComponentSignature>::operator()(ecs::ComponentSignature const& sig) noexcept
{
	auto h = sig.size();
	for (auto ID : sig)
	{
		h ^= std::hash<ecs::ComponentID>{}(ID) + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
	}
	return h;
}
