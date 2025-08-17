#pragma once
#include "field.hh"
#include <cstring>

namespace luz
{
template <typename T>
template <size_t size_bytes>
T Field<T>::value(std::span<const std::byte, size_bytes> bytes) const noexcept
{
  T val;
  // BLE and ESP32 are both little endian
  std::memcpy(&val, bytes.subspan(offset, size).data(), size);
  return val;
}

template <typename T, typename U> constexpr Field<T> offset_from(Field<U> field)
{
  return Field<T>{ field.offset + field.size };
}
} // namespace luz
