#include "protocol.hh"
#include "buffer.hh"
#include "field.hh"
#include "packet.hh"

#include <cstdio>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
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

uint8_t checksum(const std::vector<std::span<const std::byte>>& spans,
                 IndexMarker index_marker) noexcept
{
  uint8_t accumulated = static_cast<std::underlying_type_t<IndexMarker>>(index_marker);
  for (auto span : spans)
  {
    accumulated = std::accumulate(
        span.begin(), span.end(), accumulated, [](uint8_t chksm, const std::byte val) {
          return (chksm + std::to_integer<uint8_t>(val)) & 0xFF;
        });
  }
  return 0xFF & ~accumulated;
}

struct HeaderDecoder
{
  static constexpr auto first_byte_indicator_field = Field<uint8_t>{ 0U };
  static constexpr auto payload_size_field = offset_from<uint8_t>(first_byte_indicator_field);
  static constexpr auto checksum_field = offset_from<uint8_t>(payload_size_field);
  static constexpr auto second_byte_indicator_field = offset_from<uint8_t>(checksum_field);
  static constexpr auto index_marker_field = offset_from<uint8_t>(second_byte_indicator_field);
  static constexpr auto size_bytes = offset_from<uint8_t>(index_marker_field).offset;

  static bool make(std::span<const std::byte> bytes, Packet::Header& header) noexcept;
};

struct FooterDecoder
{
  static constexpr auto third_byte_indicator_field = Field<uint8_t>{ 0U };
  static constexpr auto size_bytes = offset_from<uint8_t>(third_byte_indicator_field).offset;

  static bool make(std::span<const std::byte> bytes, Packet::Footer& footer) noexcept;
};

struct PlacementDecoder
{
  static constexpr auto position_field = Field<uint16_t>{ 0U };
  static constexpr auto color_field = offset_from<uint8_t>(position_field);
  static constexpr auto size_bytes = offset_from<uint8_t>(color_field).offset;

  static bool try_iter_make(const std::vector<std::span<const std::byte>>& spans,
                            std::pmr::vector<Placement>& placements) noexcept;
  static void make(std::span<const std::byte, size_bytes> bytes, Placement& placement) noexcept;
};

struct PacketDecoder
{
  static constexpr size_t fixed_elem_size = HeaderDecoder::size_bytes + FooterDecoder::size_bytes;
  static ProtocolStatus try_make(const BufferList& buffers, Packet& packet) noexcept;
};

bool HeaderDecoder::make(std::span<const std::byte> bytes, Packet::Header& header) noexcept
{
  if (auto indicator = first_byte_indicator_field.value(bytes);
      indicator != header.first_byte_indicator)
  {
    return false;
  }

  // The index_marker is counted as part of the payload, but we've added to the header.
  header.payload_size = payload_size_field.value(bytes) - index_marker_field.size;
  header.checksum = checksum_field.value(bytes);

  if (second_byte_indicator_field.value(bytes) != header.second_byte_indicator)
  {
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

bool FooterDecoder::make(std::span<const std::byte> bytes, Packet::Footer& footer) noexcept
{
  if (third_byte_indicator_field.value(bytes) != footer.third_byte_indicator)
  {
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
}

bool PlacementDecoder::try_iter_make(const std::vector<std::span<const std::byte>>& spans,
                                     std::pmr::vector<Placement>& placements) noexcept
{
  std::array<std::byte, size_bytes> bytes{};

  auto remaining = 0UL;
  for (auto span : spans)
  {
    remaining += span.size();
    if (remaining > span.size())
    {
      // previous and current span were fragmented, splice bytes from current onto end of previous
      size_t required = size_bytes - (remaining - span.size());
      std::ranges::copy_n(span.begin(), required, bytes.end() - required);
      PlacementDecoder::make(bytes, placements.emplace_back());
      remaining -= size_bytes;
    }

    for (; remaining >= size_bytes; remaining -= size_bytes)
    {
      PlacementDecoder::make(span.subspan(span.size() - remaining).template first<size_bytes>(),
                             placements.emplace_back());
    }

    if (remaining > 0)
    {
      // current and next span were fragmented, splice bytes from current to front of next
      auto subspan = span.last(remaining);
      std::ranges::copy_n(subspan.begin(), remaining, bytes.begin());
    }
  }
  return remaining == 0UL;
}

ProtocolStatus PacketDecoder::try_make(const BufferList& buffers, Packet& packet) noexcept
{
  auto buffers_size = buffers.size();
  if (buffers_size < HeaderDecoder::size_bytes)
  {
    return ProtocolStatus::insufficient_header_bytes;
  }

  auto offset = size_t{ 0UL };
  auto header_span = buffers.span_of(offset, HeaderDecoder::size_bytes);
  if (!HeaderDecoder::make(header_span, packet.header))
  {
    return ProtocolStatus::bad_header;
  }

  if (buffers_size < (fixed_elem_size + packet.header.payload_size))
  {
    return ProtocolStatus::incomplete;
  }

  offset += HeaderDecoder::size_bytes;
  auto payload_spans = buffers.spans_of(offset, packet.header.payload_size);
  if (auto chksm = checksum(payload_spans, packet.header.index_marker);
      chksm != packet.header.checksum)
  {
    return ProtocolStatus::bad_checksum;
  }

  offset += packet.header.payload_size;
  auto footer_span = buffers.span_of(offset, FooterDecoder::size_bytes);
  if (!FooterDecoder::make(footer_span, packet.footer))
  {
    return ProtocolStatus::bad_footer;
  }

  packet.placements.reserve(detail::max_placements_per_climb);
  if (!PlacementDecoder::try_iter_make(payload_spans, packet.placements))
  {
    return ProtocolStatus::bad_payload;
  }

  return ProtocolStatus::success;
}

ProtocolStatus do_process(const BufferList& buffers, Packet& packet) noexcept
{
  return detail::PacketDecoder::try_make(buffers, packet);
}
} // namespace detail

bool Protocol::process(std::span<const std::byte> bytes, Packet& packet) noexcept
{
  buffer_list_.emplace_back(bytes.begin(), bytes.end());
  while (!buffer_list_.empty())
  {
    switch (detail::do_process(buffer_list_, packet))
    {
    case detail::ProtocolStatus::success:
    {
      buffer_list_.clear();
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
      buffer_list_.pop_front();
      break;
    }
    default:
      std::unreachable();
    }
  }
  return false;
}
} // namespace luz::protocol
