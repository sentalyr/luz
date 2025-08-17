#pragma once

#include "placement.hh"

#include <cstdint>

namespace luz::database
{
/// Number of LEDs
constexpr uint16_t num_leds = 461U;
/// Number of hold positions on the 12x12 decoy board
constexpr uint16_t num_positions = 578U;

/// Translate the placement position into the idx of the pixel array
bool placement_to_pixel(uint16_t position, uint16_t& pixel) noexcept;
} // namespace luz::database
