#include "protocol.hh"

#include <array>
#include <memory_resource>
#include <span>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace luz::protocol::test
{

const auto top_row = std::vector<std::vector<uint8_t>>{
  std::vector<uint8_t>{ 1,   52, 32, 2,   84,  17, 0,   224, 52, 0,
                        224, 87, 0,  227, 122, 0,  227, 157, 0,  227 },
  std::vector<uint8_t>{ 192, 0,   227, 227, 0,  227, 6, 1, 227, 41,
                        1,   227, 76,  1,   28, 111, 1, 3, 146, 1 },
  std::vector<uint8_t>{ 3, 181, 1, 3, 216, 1, 3, 251, 1, 227, 30, 2, 227, 65, 2, 227, 3 },
};

TEST_CASE("top row")
{
  auto protocol = Protocol{};

  std::array<std::byte, detail::max_placements_per_climb * sizeof(Placement)> memory{};
  std::pmr::monotonic_buffer_resource mem_resource{ memory.data(), memory.size() };
  std::pmr::polymorphic_allocator<int> allocator{ &mem_resource };
  std::pmr::vector<Placement> placements{ allocator };

  for (size_t i = 0; i < top_row.size(); ++i)
  {
    const auto did_parse = protocol.process(std::as_bytes(std::span(top_row[i])), placements);
    if (i < (top_row.size() - 1))
    {
      REQUIRE_FALSE(did_parse);
    }
    else
    {
      REQUIRE(did_parse);
    }
  }

  REQUIRE(placements.size() == 17UL);
  REQUIRE(placements[0].position == 17UL);

  for (size_t i = 1; i < placements.size(); ++i)
  {
    REQUIRE(placements[i].position - placements[i - 1].position == 35UL);
  }
}

constexpr auto wilbur_wright_takes_flight_p1 = std::array<std::byte, 20U>{
  std::byte{ 0x01 }, std::byte{ 0x1F }, std::byte{ 0xD6 }, std::byte{ 0x02 }, std::byte{ 0x54 },
  std::byte{ 0x29 }, std::byte{ 0x01 }, std::byte{ 0xE0 }, std::byte{ 0x6C }, std::byte{ 0x00 },
  std::byte{ 0xE3 }, std::byte{ 0x8D }, std::byte{ 0x01 }, std::byte{ 0x03 }, std::byte{ 0x12 },
  std::byte{ 0x01 }, std::byte{ 0x1C }, std::byte{ 0xAA }, std::byte{ 0x00 }, std::byte{ 0x1C }
};
constexpr auto wilbur_wright_takes_flight_p2 = std::array<std::byte, 16U>{
  std::byte{ 0xEC }, std::byte{ 0x00 }, std::byte{ 0x03 }, std::byte{ 0x0F },
  std::byte{ 0x01 }, std::byte{ 0x03 }, std::byte{ 0x34 }, std::byte{ 0x01 },
  std::byte{ 0xE3 }, std::byte{ 0x7C }, std::byte{ 0x01 }, std::byte{ 0xE3 },
  std::byte{ 0x78 }, std::byte{ 0x01 }, std::byte{ 0x03 }, std::byte{ 0x03 },
};

// I (35845) BLE:  0XEC,
// I (35845) BLE:  0,
// I (35845) BLE:  0X3,
// I (35845) BLE:  0XF,
// I (35845) BLE:  0X1,
// I (35845) BLE:  0X3,
// I (35845) BLE:  0X34,
// I (35855) BLE:  0X1,
// I (35855) BLE:  0XE3,
// I (35855) BLE:  0X7C,
// I (35855) BLE:  0X1,
// I (35855) BLE:  0XE3,
// I (35865) BLE:  0X78,
// I (35865) BLE:  0X1,
// I (35865) BLE:  0X3,
// I (35865) BLE:  0X3,
// Position = 297; RGB = 224, 0, 0
// Position = 108; RGB = 224, 0, 192
// Position = 397; RGB = 0, 0, 192
// Position = 274; RGB = 0, 224, 0
// Position = 170; RGB = 0, 224, 0
// Position = 236; RGB = 0, 0, 192
// Position = 271; RGB = 0, 0, 192
// Position = 308; RGB = 224, 0, 192
// Position = 380; RGB = 224, 0, 192
// Position = 376; RGB = 0, 0, 192

// Little Critters
// I (149954) BLE:         0XCE,
// I (149954) BLE:         0X1,
// I (149954) BLE:         0XE3,
// I (149954) BLE:         0X6,
// I (149954) BLE:         0X1,
// I (149954) BLE:         0XE0,
// I (149954) BLE:         0X57,
// I (149954) BLE:         0X1,
// I (149964) BLE:         0X3,
// I (149964) BLE:         0XF,
// I (149964) BLE:         0X1,
// I (149964) BLE:         0X3,
// I (149964) BLE:         0X7C,
// I (149974) BLE:         0X1,
// I (149974) BLE:         0X1C,
// I (149974) BLE:         0XC2,
// I (149974) BLE:         0X1,
// I (149974) BLE:         0X1C,
// I (149984) BLE:         0X3,
// Position = 327; RGB = 0, 0, 192
// Position = 366; RGB = 0, 0, 192
// Position = 433; RGB = 0, 0, 192
// Position = 245; RGB = 224, 0, 192
// Position = 322; RGB = 0, 0, 192
// Position = 462; RGB = 224, 0, 192
// Position = 262; RGB = 224, 0, 0
// Position = 343; RGB = 0, 0, 192
// Position = 271; RGB = 0, 0, 192
// Position = 380; RGB = 0, 224, 0
// Position = 450; RGB = 0, 224, 0

TEST_CASE("decode climbs")
{
  Protocol protocol{};
  auto placement_allocator = protocol.placement_allocator();
  auto placements = placement_allocator.vector();
  REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1, placements));
  REQUIRE(protocol.process(wilbur_wright_takes_flight_p2, placements));
}
} // namespace luz::protocol::test
