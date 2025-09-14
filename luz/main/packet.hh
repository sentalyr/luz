#pragma once

#include "color.hh"
#include <optional>
#include <vector>

namespace luz
{
enum class IndexMarker : uint8_t
{
  middle = 0x51,
  first = 0x52,
  last = 0x53,
  solo = 0x54,
};

constexpr std::optional<IndexMarker>
index_marker_from_underlying(std::underlying_type_t<IndexMarker> underlying) noexcept
{
  switch (underlying)
  {
  case static_cast<std::underlying_type_t<IndexMarker>>(IndexMarker::middle):
    return IndexMarker::middle;
  case static_cast<std::underlying_type_t<IndexMarker>>(IndexMarker::first):
    return IndexMarker::first;
  case static_cast<std::underlying_type_t<IndexMarker>>(IndexMarker::last):
    return IndexMarker::last;
  case static_cast<std::underlying_type_t<IndexMarker>>(IndexMarker::solo):
    return IndexMarker::solo;
  default:
    return std::nullopt;
  }
};

struct Placement
{
  uint16_t position{};
  Color color{};

  friend bool operator==(const Placement& lhs, const Placement& rhs)
  {
    return lhs.position == rhs.position && lhs.color == rhs.color;
  };
};

struct Packet
{
  struct Header
  {
    static constexpr uint8_t first_byte_indicator = 0x01;
    uint8_t payload_size{};
    uint8_t checksum{};
    static constexpr uint8_t second_byte_indicator = 0x02;
    IndexMarker index_marker{};
  };

  struct Footer
  {
    static constexpr uint8_t third_byte_indicator = 0x03;
  };

  Header header{};
  std::pmr::vector<Placement> placements;
  Footer footer{};
};
} // namespace luz
