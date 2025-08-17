#pragma once
#include <cstddef>
#include <span>

namespace luz
{
template <typename T> struct Field
{
  using value_type = T;

  size_t offset{ 0UL };
  static constexpr size_t size = sizeof(T);

  template <size_t size_bytes>
  T value(std::span<const std::byte, size_bytes> bytes) const noexcept;
};

template <typename T, typename U> constexpr Field<T> offset_from(Field<U> field);
}

#include "field.inl"
