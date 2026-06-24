#pragma once

#include "type_id.hpp"

namespace ecs
{

class ComponentSignature
{
  public:
	ComponentSignature() = default;

	template <typename... Ts> static ComponentSignature Create();
	void												Insert(ComponentID ID);
	void												Erase(ComponentID ID);
	bool												Contains(ComponentID ID);
	bool												ContainsAll(ComponentSignature const& other) const;

  public:
	std::size_t size() const noexcept;
	bool		empty() const noexcept;

  public:
	friend bool operator==(ComponentSignature const&, ComponentSignature const&) = default;

  private:
	std::vector<ComponentID> m_IDs;

  public:
	using const_iterator = decltype(m_IDs)::const_iterator;

	const_iterator begin() const noexcept;
	const_iterator end() const noexcept;
};

template <typename... Ts> ComponentSignature ComponentSignature::Create()
{
	auto sig = ComponentSignature{};
	(sig.Insert(GetComponentID<Ts>()), ...);
	return sig;
}

} // namespace ecs

template <> struct std::hash<ecs::ComponentSignature>
{
	static std::size_t operator()(ecs::ComponentSignature const& sig) noexcept;
};
