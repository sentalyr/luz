#include "ble.hh"
#include "color.hh"
#include "database.hh"
#include "led.hh"
#include "placement.hh"
#include "protocol.hh"

#include <algorithm>
#include <ranges>
#include <span>

namespace
{
constexpr auto peripheral_name = "Steve's Decoy Board";
constexpr auto tag = "LUZ";

constexpr uint16_t ms(uint16_t millis) noexcept { return millis / portTICK_PERIOD_MS; }

// Function object invoked for each write to the DecoyPeripheral characteristic
class OnWrite
{
public:
  OnWrite() noexcept = default;
  ~OnWrite() noexcept = default;

  /// Copy/move constructor/assignment
  OnWrite(const OnWrite&) = delete;
  OnWrite& operator=(const OnWrite&) = delete;
  OnWrite(OnWrite&&) = delete;
  OnWrite& operator=(OnWrite&&) = delete;

  /// Call operator invoked each time the DecoyPeripheral characteristic is
  /// written to by a client
  /// @param bytes The payload written by the client
  void operator()(std::span<const std::byte> bytes) noexcept
  {
    auto placement_allocator = protocol_.placement_allocator();
    auto placements = placement_allocator.vector();
    if (protocol_.process(bytes, placements))
    {
      ESP_LOGD(tag, "OnWrite: Recieved a packet!");

      leds_.clear();

      std::ranges::for_each(placements, [this](const auto& placement) {
        ESP_LOGD(tag,
                 "Placement: %d: Color(r=%#X, g=%#X, b=%#X)",
                 placement.position,
                 placement.color.r,
                 placement.color.g,
                 placement.color.b);

        uint16_t pixel_idx;
        if (!luz::database::placement_to_pixel(placement.position, pixel_idx))
        {
          ESP_LOGE(tag,
                   "Invalid placement position %u; cannot convert to "
                   "pixel index!",
                   placement.position);
          indicate_failure();
        }
        ESP_LOGD(tag, "Setting placement position %u to pixel %u", placement.position, pixel_idx);
        leds_.set_pixel(pixel_idx, placement.color);
      });

      leds_.submit();
    }
  };

private:
  /// Flash every tenth pixel red for 20s and terminate
  void indicate_failure()
  {
    constexpr uint16_t num_100ms_cycles = 200U;
    constexpr auto red = luz::Color(0U, 255U, 0U);

    for (uint16_t i = 0U; i < num_100ms_cycles; ++i)
    {
      leds_.clear();
      vTaskDelay(ms(100U));
      for (uint32_t pxl = 0; pxl < luz::database::num_leds; pxl += 10)
      {
        leds_.set_pixel(pxl, red);
      }
      leds_.submit();
      vTaskDelay(ms(100U));
    }

    std::abort();
  };

  luz::led::ESP32LED leds_{ luz::database::num_leds };
  luz::protocol::Protocol protocol_{};
};
} // anonymous namespace

extern "C" void app_main(void)
{
  auto on_write = OnWrite{};
  auto decoy_peripheral = luz::ble::DecoyPeripheral{ peripheral_name, on_write };
  (void)decoy_peripheral;

  ESP_LOGI(tag, "Decoy Peripheral created");

  while (true)
  {
    vTaskDelay(ms(2000));
  }
}
