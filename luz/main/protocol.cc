#include "protocol.hh"
#include "field.hh"

#include <cstdio>
#include <memory>
#include <numeric>
#include <vector>

namespace luz::protocol
{
namespace detail
{
struct PacketByteIndicator
{
  static constexpr uint8_t first = 0x01;
  static constexpr uint8_t second = 0x02;
  static constexpr uint8_t third = 0x03;
};

struct IndexMarker
{
  using value_type = uint8_t;
  static constexpr uint8_t middle = 0x51;
  static constexpr uint8_t first = 0x52;
  static constexpr uint8_t last = 0x53;
  static constexpr uint8_t solo = 0x54;
};

uint8_t checksum(std::span<const std::byte> bytes, IndexMarker::value_type index_marker) noexcept
{
  return 0xFF
         & ~(std::accumulate(
             bytes.begin(), bytes.end(), index_marker, [](uint8_t chksm, std::byte val) {
               return (chksm + std::to_integer<uint8_t>(val)) & 0xFF;
             }));
}

struct PacketHeader
{
  static constexpr auto first_byte_indicator_field = Field<uint8_t>{ 0U };
  static constexpr auto payload_size_field = offset_from<uint8_t>(first_byte_indicator_field);
  static constexpr auto checksum_field = offset_from<uint8_t>(payload_size_field);
  static constexpr auto second_byte_indicator_field = offset_from<uint8_t>(checksum_field);
  static constexpr auto index_marker_field = offset_from<uint8_t>(second_byte_indicator_field);
  static constexpr auto size_bytes = offset_from<uint8_t>(index_marker_field).offset;

  static void make(std::span<const std::byte, size_bytes> bytes, PacketHeader& header) noexcept;

  uint8_t first_byte_indicator{};
  uint8_t payload_size{};
  uint8_t checksum{};
  uint8_t second_byte_indicator{};
  uint8_t index_marker{};
};

struct PacketFooter
{
  static constexpr auto third_byte_indicator_field = Field<uint8_t>{ 0U };
  static constexpr auto size_bytes = offset_from<uint8_t>(third_byte_indicator_field).offset;

  static void make(std::span<const std::byte, size_bytes> bytes, PacketFooter& footer) noexcept;

  uint8_t third_byte_indicator{};
};

struct PlacementField
{
  static constexpr auto position_field = Field<uint16_t>{ 0U };
  static constexpr auto color_field = offset_from<uint8_t>(position_field);
  static constexpr auto size_bytes = offset_from<uint8_t>(color_field).offset;

  static bool try_iter_make(std::span<const std::byte> bytes,
                            std::pmr::vector<Placement>& placements) noexcept;
  static void make(std::span<const std::byte, size_bytes> bytes, Placement& placement) noexcept;
};

class Packet
{
  static constexpr size_t fixed_elem_size = PacketHeader::size_bytes + PacketFooter::size_bytes;

public:
  Packet(std::pmr::vector<Placement>& _placements) noexcept;
  ~Packet() noexcept = default;

  /// Copy/move constructor/assignment
  Packet(const Packet&) = delete;
  Packet& operator=(const Packet&) = delete;
  Packet(Packet&&) = delete;
  Packet& operator=(Packet&&) = delete;

  static bool try_make(std::span<const std::byte> bytes, Packet& packet) noexcept;

  PacketHeader header{};
  std::pmr::vector<Placement>* placements;
  PacketFooter footer{};
};

void PacketHeader::make(std::span<const std::byte, PacketHeader::size_bytes> bytes,
                        PacketHeader& header) noexcept
{
  header.first_byte_indicator = first_byte_indicator_field.value(bytes);

  // The index_marker is counted as part of the payload, but we've added to the header.
  header.payload_size = payload_size_field.value(bytes) - index_marker_field.size;

  printf("HEADER payload size %d, %d", payload_size_field.value(bytes), index_marker_field.size);

  header.checksum = checksum_field.value(bytes);
  header.second_byte_indicator = second_byte_indicator_field.value(bytes);
  header.index_marker = index_marker_field.value(bytes);
}

void PacketFooter::make(std::span<const std::byte, PacketFooter::size_bytes> bytes,
                        PacketFooter& footer) noexcept
{
  footer.third_byte_indicator = third_byte_indicator_field.value(bytes);
}

void PlacementField::make(std::span<const std::byte, PlacementField::size_bytes> bytes,
                          Placement& placement) noexcept
{
  placement.position = position_field.value(bytes);

  const auto color = color_field.value(bytes);
  placement.color.r = ((color & 0b11100000) >> 5) * 32;
  placement.color.g = ((color & 0b00011100) >> 2) * 32;
  placement.color.b = (color & 0b00000011) * 64;

  printf("Position = %u; RGB = %u, %u, %u\n",
         static_cast<uint32_t>(placement.position),
         static_cast<uint32_t>(placement.color.r),
         static_cast<uint32_t>(placement.color.g),
         static_cast<uint32_t>(placement.color.b));
}

bool PlacementField::try_iter_make(std::span<const std::byte> bytes,
                                   std::pmr::vector<Placement>& placements) noexcept
{
  if ((bytes.size() % PlacementField::size_bytes) != 0UL)
  {
    printf("Payload size must be a multiple of Placement size; %zu mod %zu != 0\n",
           bytes.size(),
           PlacementField::size_bytes);
    return false;
  }

  for (size_t bytes_idx = 0UL; bytes_idx < bytes.size(); bytes_idx += PlacementField::size_bytes)
  {
    PlacementField::make(bytes.subspan(bytes_idx).template first<PlacementField::size_bytes>(),
                         placements.emplace_back());
  }

  return true;
}

Packet::Packet(std::pmr::vector<Placement>& _placements) noexcept
    : placements{ std::pointer_traits<std::pmr::vector<Placement>*>::pointer_to(_placements) }
{
}

bool Packet::try_make(std::span<const std::byte> bytes, Packet& packet) noexcept
{
  if (bytes.size() < PacketHeader::size_bytes)
  {
    printf("Not enough bytes for header\n");
    return false;
  }

  PacketHeader::make(bytes.template subspan<0, PacketHeader::size_bytes>(), packet.header);

  if (bytes.size() < (fixed_elem_size + packet.header.payload_size))
  {
    printf("Not enough bytes for payload and footer %d < %d\n",
           bytes.size(),
           (fixed_elem_size + packet.header.payload_size));
    return false;
  }

  const auto payload_bytes = bytes.subspan(PacketHeader::size_bytes, packet.header.payload_size);
  packet.placements->reserve(detail::max_placements_per_climb);
  if (!PlacementField::try_iter_make(payload_bytes, *packet.placements))
  {
    printf("Failed to create placements\n");
    return false;
  }

  const auto footer_bytes = bytes.subspan(PacketHeader::size_bytes + packet.header.payload_size)
                                .template first<PacketFooter::size_bytes>();
  PacketFooter::make(footer_bytes, packet.footer);

  if (packet.header.first_byte_indicator != PacketByteIndicator::first)
  {
    printf("Invalid first byte indicator: %zu\n", packet.header.first_byte_indicator);
    return false;
  }
  if (packet.header.second_byte_indicator != PacketByteIndicator::second)
  {
    printf("Invalid second byte indicator: %zu\n", packet.header.second_byte_indicator);
    return false;
  }
  if (packet.footer.third_byte_indicator != PacketByteIndicator::third)
  {
    printf("Invalid third byte indicator: %zu\n", packet.footer.third_byte_indicator);
    return false;
  }

  if (auto chksm = checksum(payload_bytes, packet.header.index_marker);
      chksm != packet.header.checksum)
  {
    printf("Checksum mismatch %zu != %zu\n", chksm, packet.header.checksum);
    return false;
  }

  return true;
}
} // namespace detail

bool Protocol::process(std::span<const std::byte> bytes,
                       std::pmr::vector<Placement>& placements) noexcept
{
  std::copy(bytes.begin(), bytes.end(), std::back_inserter(buffer_));

  auto packet = detail::Packet{ placements };
  if (!detail::Packet::try_make(buffer_, packet))
  {
    return false;
  }

  buffer_.clear();

  switch (packet.header.index_marker)
  {
  case detail::IndexMarker::solo:
    return true;
  default:
  {
    // TODO handle cases where climb can't fit in single BLE packet
    // case detail::IndexMarker::middle:
    // case detail::IndexMarker::first:
    // case detail::IndexMarker::last:
    printf("FAILURE: index_marker != solo unimplemented");
    return false;
  }
  }
}
} // namespace luz::protocol
