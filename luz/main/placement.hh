#pragma once

#include "color.hh"

namespace luz
{
struct Placement
{
  uint16_t position{};
  Color color{};

  friend bool operator==(const Placement& lhs, const Placement& rhs)
  {
    return lhs.position == rhs.position && lhs.color == rhs.color;
  };
};
} // namespace luz
