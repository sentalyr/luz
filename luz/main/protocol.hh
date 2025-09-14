#pragma once

#include "packet.hh"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <list>
#include <memory_resource>
#include <span>
#include <vector>

namespace luz::protocol
{
namespace detail
{
static constexpr size_t default_buffer_size = 512UL;
static constexpr size_t max_placements_per_climb = 35UL;

template <typename Elem, size_t NumElems> struct MonotonicAllocator
{
  using Vector = std::pmr::vector<Elem>;

  MonotonicAllocator(std::span<Elem, NumElems> memory) noexcept
      : mem_resource{ memory.data(), memory.size() }, allocator{ &mem_resource }
  {
  }

  ~MonotonicAllocator() noexcept = default;

  /// Copy/move constructor/assignment
  MonotonicAllocator(const MonotonicAllocator&) = delete;
  MonotonicAllocator& operator=(const MonotonicAllocator&) = delete;
  MonotonicAllocator(MonotonicAllocator&&) = delete;
  MonotonicAllocator& operator=(MonotonicAllocator&&) = delete;

  Vector vector() noexcept { return std::pmr::vector<Elem>{ allocator }; }

  Vector capped_vector() noexcept
  {
    auto v = std::pmr::vector<Elem>{ allocator };
    v.reserve(NumElems);
    return v;
  }

  std::pmr::monotonic_buffer_resource mem_resource;
  std::pmr::polymorphic_allocator<Elem> allocator;
};

/// Deduction guide
template <typename Elem, size_t NumElems>
MonotonicAllocator(std::array<Elem, NumElems>) -> MonotonicAllocator<Elem, NumElems>;

} // namespace detail

class Protocol
{
public:
  using PlacementAllocator
      = detail::MonotonicAllocator<Placement, detail::max_placements_per_climb>;

  Protocol() noexcept = default;
  ~Protocol() noexcept = default;

  /// Copy/move constructor/assignment
  Protocol(const Protocol&) = delete;
  Protocol& operator=(const Protocol&) = delete;
  Protocol(Protocol&&) = delete;
  Protocol& operator=(Protocol&&) = delete;

  /// Process an incoming payload and attempt to extract a complete set of placements
  /// @param bytes The incoming byte payload
  /// @param[out] placements The set of placements if parsing the incoming payload completes the
  /// set
  /// @return Boolean indicating if the set of placements is valid
  bool process(std::span<const std::byte> bytes, Packet& packet) noexcept;

  PlacementAllocator placement_allocator() noexcept
  {
    return PlacementAllocator{ placement_memory_ };
  }

private:
  std::array<std::byte, detail::default_buffer_size> buffer_memory_{};
  detail::MonotonicAllocator<std::byte, detail::default_buffer_size> buffer_alloc_{
    buffer_memory_
  };
  decltype(buffer_alloc_)::Vector buffer_{ buffer_alloc_.capped_vector() };

  std::array<Placement, detail::max_placements_per_climb> placement_memory_{};

  std::list<std::vector<std::byte>> buffers_{};
};
} // namespace luz::protocol

#include "protocol.inl"
