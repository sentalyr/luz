#include "led.hh"

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

#include <stdio.h>

namespace luz::led
{
namespace
{
// Let the driver choose a proper memory block size automatically
constexpr size_t memory_block_words_size = 0UL;
// 10MHz resolution, 1 tick = 0.1us
constexpr size_t rmt_resolution_hz = 10'000'000;
// Logging tag
constexpr auto tag = "luz::led";

led_strip_handle_t configure_led(uint32_t num_leds, uint8_t gpio_pin) noexcept
{
  led_strip_config_t strip_config = { .strip_gpio_num = gpio_pin,
                                      .max_leds = num_leds,
                                      .led_model = LED_MODEL_WS2811,
                                      .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
                                      .flags = {
                                          .invert_out = false,
                                      } };

  led_strip_rmt_config_t rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = rmt_resolution_hz,
    .mem_block_symbols = memory_block_words_size,
    .flags = {},
  };

  led_strip_handle_t led_strip;
  ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
  ESP_LOGI(tag, "Created LED strip object with RMT backend");
  return led_strip;
}
} // anonymous namespace

ESP32LED::ESP32LED(uint32_t num_leds, uint8_t gpio_pin) noexcept
    : led_strip_{ configure_led(num_leds, gpio_pin) }
{
}

void ESP32LED::set_pixel(uint32_t idx, Color color) noexcept
{
  ESP_ERROR_CHECK(led_strip_set_pixel(led_strip_, idx, color.g, color.r, color.b));
}

void ESP32LED::submit() noexcept { ESP_ERROR_CHECK(led_strip_refresh(led_strip_)); }

void ESP32LED::clear() noexcept { ESP_ERROR_CHECK(led_strip_clear(led_strip_)); }
} // namespace luz::led
