#pragma once

#include <cstdint>

namespace luz
{
/// Simple struct representing an RGB color
struct Color
{
  uint8_t r = 0U;
  uint8_t g = 0U;
  uint8_t b = 0U;

  friend bool operator==(const Color& lhs, const Color& rhs)
  {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
  };
};
} // namespace luz
