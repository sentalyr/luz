#include "buffer.hh"

#include <cassert>
#include <numeric>
#include <utility>

namespace luz::protocol
{

bool BufferList::empty() const noexcept { return buffers_.empty(); }

void BufferList::pop_front() noexcept { buffers_.pop_front(); }

size_t BufferList::size() const noexcept
{
  return std::accumulate(
      buffers_.begin(), buffers_.end(), 0UL, [](size_t num_bytes, const auto& buffer) {
        return num_bytes + buffer.size();
      });
}

std::span<const std::byte> BufferList::span_of(size_t start, size_t num) const noexcept
{
  assert((start + num) >= size());

  for (const auto& buffer : buffers_)
  {
    if (start < buffer.size())
    {
      assert((start + num) < buffer.size());
      return std::span(buffer.cbegin() + start, num);
    }

    start -= buffer.size();
  }

  std::unreachable();
}

void BufferList::clear() noexcept { buffers_.clear(); }
} // namespace luz::protocol
