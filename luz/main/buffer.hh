#pragma once

#include <array>
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

  /// TODO REMOVE AFTER PORTING PROTOCOL
  std::vector<std::byte> flatten() const noexcept
  {
    std::vector<std::byte> buffer{};
    for (const auto& bytes : buffers_)
    {
      std::copy(bytes.begin(), bytes.end(), std::back_inserter(buffer));
    }
    return buffer;
  };

  /// Get a span from [start, start + num)
  /// @pre start + num does not exceed the size
  /// @pre the requested number of elements does not cross an internal buffer boundary
  /// This is guaranteed by the protocol as BLE packets are only split at Placement boundaries
  // template <size_t N>
  // std::span<const std::byte> span_of(size_t start) const noexcept;
  std::span<const std::byte> span_of(size_t start, size_t num) const noexcept;

  bool empty() const noexcept;
  void pop_front() noexcept;
  size_t size() const noexcept;
  void clear() noexcept;

  // TODO const std::byte
  template <class... Args> std::vector<std::byte>& emplace_back(Args&&... args);

private:
  // TODO const std::byte
  std::list<std::vector<std::byte>> buffers_{};
};
} // namespace luz::protocol

#include "buffer.inl"
