#pragma once

#include <array>
#include <iterator>
#include <list>
#include <memory_resource>
#include <span>
#include <vector>

namespace luz::protocol
{
class BufferList
{
public:
  BufferList() noexcept = default;
  ~BufferList() noexcept = default;

  /// Copy/move constructor/assignment
  BufferList(const BufferList&) = delete;
  BufferList& operator=(const BufferList&) = delete;
  BufferList(BufferList&&) = delete;
  BufferList& operator=(BufferList&&) = delete;

  /// Get a span from [start, start + num)
  /// @pre start + num does not exceed the size
  /// @pre the requested number of elements does not cross an internal buffer boundary
  /// This is guaranteed by the protocol as BLE packets are only split at Placement boundaries
  // TODO add templated version
  // template <size_t N>
  // std::span<const std::byte, N> span_of(size_t start) const noexcept;
  std::span<const std::byte> span_of(size_t start, size_t num) const noexcept;

  /// Get a span of spans with sum total 'num' bytes where the returned number of spans is dictated
  /// by the number of internal buffer boundaries crossed
  std::vector<std::span<const std::byte>> spans_of(size_t start, size_t num) const noexcept;

  bool empty() const noexcept;
  void pop_front() noexcept;
  size_t size() const noexcept;
  void clear() noexcept;

  template <class... Args> std::vector<std::byte>& emplace_back(Args&&... args);

private:
  std::list<std::vector<std::byte>> buffers_{};
};
} // namespace luz::protocol

#include "buffer.inl"
