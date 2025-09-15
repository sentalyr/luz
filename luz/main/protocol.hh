#pragma once

#include "buffer.hh"
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
static constexpr size_t max_placements_per_climb = 35UL;
} // namespace detail

class Protocol
{
public:
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

private:
  BufferList buffer_list_{};
};
} // namespace luz::protocol
