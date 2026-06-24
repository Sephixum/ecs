#include <ecs/column.hpp>
#include <new>
#include <ranges>
#include <utility>

namespace ecs
{

namespace vws = std::views;

Column::Column(ComponentInfo const& info) : m_info{&info} {}

Column::Column(Column&& other)
{
	*this = std::move(other);
}

Column& Column::operator=(Column&& other)
{
	if (this == &other)
	{
		return *this;
	}

	ReleaseStorage();
	m_info	   = std::exchange(other.m_info, nullptr);
	m_data	   = std::exchange(other.m_data, nullptr);
	m_count	   = std::exchange(other.m_count, 0);
	m_capacity = std::exchange(other.m_capacity, 0);

	return *this;
}

Column::~Column()
{
	ReleaseStorage();
}

void Column::Reserve(std::size_t new_capacity)
{
	if (new_capacity <= m_capacity)
	{
		return;
	}

	auto* new_data =
		static_cast<std::byte*>(::operator new(new_capacity * m_info->size, std::align_val_t{m_info->align}));

	for (auto i : vws::iota(0zu, m_count))
	{
		m_info->move_construct(new_data + i * m_info->size, m_data + i * m_info->size);
		m_info->destroy(m_data + i * m_info->size);
	}

	if (m_data)
	{
		::operator delete(m_data, std::align_val_t{m_info->align});
	}

	m_data	   = new_data;
	m_capacity = new_capacity;
}

std::size_t Column::PushDefaultConstructed()
{
	EnsureCapactiy(m_count + 1);
	m_info->default_construct(m_data + m_count * m_info->size);
	return m_count++;
}

void Column::MoveAssignFrom(std::size_t dst_row, Column& src, std::size_t src_row)
{
	m_info->move_assign(GetAt(dst_row), src.GetAt(src_row));
}

void Column::SwapRemove(std::size_t row)
{
	auto* slot = static_cast<std::byte*>(GetAt(row));
	m_info->destroy(slot);
	auto last = m_count - 1;

	if (row != last)
	{
		auto* last_slot = static_cast<std::byte*>(GetAt(last));
		m_info->move_construct(slot, last_slot);
		m_info->destroy(last_slot);
	}

	m_count--;
}

void* Column::GetAt(std::size_t row)
{
	return m_data + row * m_info->size;
}

void const* Column::GetAt(std::size_t row) const
{
	return m_data + row * m_info->size;
}

std::size_t Column::GetCount() const noexcept
{
	return m_count;
}

ComponentInfo const& Column::GetInfo() const noexcept
{
	return *m_info;
}

void Column::EnsureCapactiy(std::size_t needed)
{
	if (needed <= m_capacity)
	{
		return;
	}

	Reserve(m_capacity == 0 ? 8 : m_capacity * 2);
}

void Column::ReleaseStorage()
{
	if (not m_data)
	{
		return;
	}

	for (auto i : vws::iota(0zu, m_count))
	{
		m_info->destroy(m_data + i * m_info->size);
	}

	::operator delete(m_data, std::align_val_t{m_info->align});

	m_data = nullptr;
}

} // namespace ecs
