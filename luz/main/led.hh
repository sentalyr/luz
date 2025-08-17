#pragma once

#include "color.hh"

#include "led_strip_types.h"

namespace luz::led
{

class ESP32LED
{
  /// The default output pin to which the WS2811 data wire is connected
  static constexpr uint8_t default_gpio_pin = 2U;

public:
  ESP32LED(uint32_t num_leds, uint8_t gpio_pin = default_gpio_pin) noexcept;
  ~ESP32LED() noexcept = default;

  /// Copy/move constructor/assignment
  ESP32LED(const ESP32LED&) = delete;
  ESP32LED& operator=(const ESP32LED&) = delete;
  ESP32LED(ESP32LED&&) = delete;
  ESP32LED& operator=(ESP32LED&&) = delete;

  /// Set the color of an individual pixel
  /// @param idx The index of the pixel to set within the LED strip
  /// @param color The color assigned to the pixel
  void set_pixel(uint32_t idx, Color color) noexcept;

  /// Submit pending changes to the LED strip
  void submit() noexcept;

  /// Clear all pixels in the LED strip
  void clear() noexcept;

private:
  led_strip_handle_t led_strip_{};
};
} // namespace luz::led
