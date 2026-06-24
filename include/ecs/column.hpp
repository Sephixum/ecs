#pragma once

#include "type_id.hpp"
#include <expected>

namespace ecs
{

class Column
{
  public:
	Column() = default;
	explicit Column(ComponentInfo const& info);
	Column(Column const&)			 = delete;
	Column& operator=(Column const&) = delete;
	Column(Column&& other);
	Column& operator=(Column&& other);
	~Column();

  public:
	void		Reserve(std::size_t new_capacity);
	std::size_t PushDefaultConstructed();
	void		MoveAssignFrom(std::size_t dst_row, Column& src, std::size_t src_row);
	void		SwapRemove(std::size_t row);

  public:
	void*				 GetAt(std::size_t row);
	void const*			 GetAt(std::size_t row) const;
	std::size_t			 GetCount() const noexcept;
	ComponentInfo const& GetInfo() const noexcept;

	template <typename T> T* ViewAs();

  private:
	void EnsureCapactiy(std::size_t needed);
	void ReleaseStorage();

  private:
	ComponentInfo const* m_info		= nullptr;
	std::byte*			 m_data		= nullptr;
	std::size_t			 m_count	= 0;
	std::size_t			 m_capacity = 0;
};

template <typename T> T* Column::ViewAs()
{
	return static_cast<T*>(static_cast<void*>(m_data));
}

} // namespace ecs
