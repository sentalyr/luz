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
  size_t size = 0;
  for (const auto& buffer : buffers_)
  {
    size += buffer.size();
  }
  return size;
}

std::span<const std::byte> BufferList::span_of(size_t start, size_t num) const noexcept
{
  assert((start + num) <= size());

  for (const auto& buffer : buffers_)
  {
    if (start < buffer.size())
    {
      assert((start + num) <= buffer.size());
      return std::span(buffer.begin() + start, num);
    }

    start -= buffer.size();
  }

  printf("span of unreachable");
  return {};
  std::unreachable();
}

std::vector<std::span<const std::byte>> BufferList::spans_of(size_t start,
                                                             size_t num) const noexcept
{
  assert((start + num) <= size());
  auto spans = std::vector<std::span<const std::byte>>{};
  for (const auto& buffer : buffers_)
  {
    if (start >= buffer.size())
    {
      start -= buffer.size();
      continue;
    }

    auto consume = std::min(buffer.size() - start, num);
    spans.push_back(std::span(buffer.cbegin() + start, consume));
    num -= consume;
    start = 0;
    if (num == 0)
    {
      return spans;
    }
  }

  printf("spans of unreachable");
  return {};
  // std::unreachable();
}

void BufferList::clear() noexcept { buffers_.clear(); }
} // namespace luz::protocol
