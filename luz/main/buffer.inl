#pragma once

#include "buffer.hh"

namespace luz::protocol
{
template <class... Args> std::vector<std::byte>& BufferList::emplace_back(Args&&... args)
{
  return buffers_.emplace_back(std::forward<Args>(args)...);
}
} // namespace luz::protocol
