#include "protocol.hh"
#include "field.hh"
#include "packet.hh"

#include <cstdio>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace luz::protocol
{
namespace detail
{

enum class ProtocolStatus
{
  success = 0,
  incomplete,
  insufficient_header_bytes,
  bad_header,
  bad_payload,
  bad_footer,
  bad_checksum,
};

uint8_t checksum(std::span<const std::byte> bytes, IndexMarker index_marker) noexcept
{
  return 0xFF
         & ~(std::accumulate(bytes.begin(),
                             bytes.end(),
                             static_cast<std::underlying_type_t<IndexMarker>>(index_marker),
                             [](uint8_t chksm, std::byte val) {
                               return (chksm + std::to_integer<uint8_t>(val)) & 0xFF;
                             }));
}

struct HeaderDecoder
{
  static constexpr auto first_byte_indicator_field = Field<uint8_t>{ 0U };
  static constexpr auto payload_size_field = offset_from<uint8_t>(first_byte_indicator_field);
  static constexpr auto checksum_field = offset_from<uint8_t>(payload_size_field);
  static constexpr auto second_byte_indicator_field = offset_from<uint8_t>(checksum_field);
  static constexpr auto index_marker_field = offset_from<uint8_t>(second_byte_indicator_field);
  static constexpr auto size_bytes = offset_from<uint8_t>(index_marker_field).offset;

  static bool make(std::span<const std::byte, size_bytes> bytes, Packet::Header& header) noexcept;
};

struct FooterDecoder
{
  static constexpr auto third_byte_indicator_field = Field<uint8_t>{ 0U };
  static constexpr auto size_bytes = offset_from<uint8_t>(third_byte_indicator_field).offset;

  static bool make(std::span<const std::byte, size_bytes> bytes, Packet::Footer& footer) noexcept;
};

struct PlacementDecoder
{
  static constexpr auto position_field = Field<uint16_t>{ 0U };
  static constexpr auto color_field = offset_from<uint8_t>(position_field);
  static constexpr auto size_bytes = offset_from<uint8_t>(color_field).offset;

  static bool try_iter_make(std::span<const std::byte> bytes,
                            std::pmr::vector<Placement>& placements) noexcept;
  static void make(std::span<const std::byte, size_bytes> bytes, Placement& placement) noexcept;
};

struct PacketDecoder
{
  static constexpr size_t fixed_elem_size = HeaderDecoder::size_bytes + FooterDecoder::size_bytes;
  static ProtocolStatus try_make(std::span<const std::byte> bytes, Packet& packet) noexcept;
};

bool HeaderDecoder::make(std::span<const std::byte, size_bytes> bytes,
                         Packet::Header& header) noexcept
{
  if (first_byte_indicator_field.value(bytes) != header.first_byte_indicator)
  {
    // printf("Invalid first byte indicator: %zu\n", header.first_byte_indicator);
    return false;
  }

  // The index_marker is counted as part of the payload, but we've added to the header.
  header.payload_size = payload_size_field.value(bytes) - index_marker_field.size;
  header.checksum = checksum_field.value(bytes);

  if (second_byte_indicator_field.value(bytes) != header.second_byte_indicator)
  {
    // printf("Invalid second byte indicator: %zu\n", header.second_byte_indicator);
    return false;
  }

  auto maybe_index_marker = index_marker_from_underlying(index_marker_field.value(bytes));
  if (!maybe_index_marker)
  {
    return false;
  }
  header.index_marker = *maybe_index_marker;
  return true;
}

bool FooterDecoder::make(std::span<const std::byte, size_bytes> bytes,
                         Packet::Footer& footer) noexcept
{
  if (third_byte_indicator_field.value(bytes) != footer.third_byte_indicator)
  {
    // printf("Invalid third byte indicator: %zu\n", footer.third_byte_indicator);
    return false;
  }
  return true;
}

void PlacementDecoder::make(std::span<const std::byte, size_bytes> bytes,
                            Placement& placement) noexcept
{
  placement.position = position_field.value(bytes);

  const auto color = color_field.value(bytes);
  placement.color.r = ((color & 0b11100000) >> 5) * 32;
  placement.color.g = ((color & 0b00011100) >> 2) * 32;
  placement.color.b = (color & 0b00000011) * 64;

  // printf("Position = %u; RGB = %u, %u, %u\n",
  //        static_cast<uint32_t>(placement.position),
  //        static_cast<uint32_t>(placement.color.r),
  //        static_cast<uint32_t>(placement.color.g),
  //        static_cast<uint32_t>(placement.color.b));
}

bool PlacementDecoder::try_iter_make(std::span<const std::byte> bytes,
                                     std::pmr::vector<Placement>& placements) noexcept
{
  if ((bytes.size() % size_bytes) != 0UL)
  {
    // printf("Payload size must be a multiple of Placement size; %zu mod %zu != 0\n",
    //        bytes.size(),
    //        PlacementField::size_bytes);
    return false;
  }

  for (size_t bytes_idx = 0UL; bytes_idx < bytes.size(); bytes_idx += size_bytes)
  {
    PlacementDecoder::make(bytes.subspan(bytes_idx).template first<size_bytes>(),
                           placements.emplace_back());
  }

  return true;
}

ProtocolStatus PacketDecoder::try_make(std::span<const std::byte> bytes, Packet& packet) noexcept
{
  if (bytes.size() < HeaderDecoder::size_bytes)
  {
    return ProtocolStatus::insufficient_header_bytes;
  }

  if (!HeaderDecoder::make(bytes.template subspan<0, HeaderDecoder::size_bytes>(), packet.header))
  {
    return ProtocolStatus::bad_header;
  }

  if (bytes.size() < (fixed_elem_size + packet.header.payload_size))
  {
    // printf("Not enough bytes for payload and footer %d < %d\n",
    //        bytes.size(),
    //        (fixed_elem_size + packet.header.payload_size));
    return ProtocolStatus::incomplete;
  }

  const auto payload_bytes = bytes.subspan(HeaderDecoder::size_bytes, packet.header.payload_size);
  // TODO enum to uint8_t
  if (auto chksm = checksum(payload_bytes, packet.header.index_marker);
      chksm != packet.header.checksum)
  {
    // printf("Checksum mismatch %zu != %zu\n", chksm, packet.header.checksum);
    return ProtocolStatus::bad_checksum;
  }

  const auto footer_bytes = bytes.subspan(HeaderDecoder::size_bytes + packet.header.payload_size)
                                .template first<FooterDecoder::size_bytes>();
  if (!FooterDecoder::make(footer_bytes, packet.footer))
  {
    return ProtocolStatus::bad_footer;
  }

  packet.placements.reserve(detail::max_placements_per_climb);
  if (!PlacementDecoder::try_iter_make(payload_bytes, packet.placements))
  {
    return ProtocolStatus::bad_payload;
  }

  return ProtocolStatus::success;
}

ProtocolStatus do_process(const std::list<std::vector<std::byte>>& buffers,
                          Packet& packet) noexcept
{
  std::vector<std::byte> buffer{};
  for (const auto& bytes : buffers)
  {
    std::copy(bytes.begin(), bytes.end(), std::back_inserter(buffer));
  }
  return detail::PacketDecoder::try_make(buffer, packet);
}
} // namespace detail

bool Protocol::process(std::span<const std::byte> bytes, Packet& packet) noexcept
{
  buffers_.emplace_back(bytes.begin(), bytes.end());
  while (!buffers_.empty())
  {
    switch (detail::do_process(buffers_, packet))
    {
    case detail::ProtocolStatus::success:
    {
      buffers_.clear();
      return true;
    }
    case detail::ProtocolStatus::incomplete:
    {
      /// Wait and accumulate additional packets
      return false;
    }
    case detail::ProtocolStatus::bad_header:
    case detail::ProtocolStatus::insufficient_header_bytes:
    case detail::ProtocolStatus::bad_payload:
    case detail::ProtocolStatus::bad_footer:
    case detail::ProtocolStatus::bad_checksum:
    {
      /// Remove the oldest and try to interpret remaining as a Packet
      buffers_.pop_front();
      break;
    }
    default:
      std::unreachable();
    }
  }
  return false;
}
} // namespace luz::protocol
