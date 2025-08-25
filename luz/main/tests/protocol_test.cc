#include "protocol.hh"

#include <array>
#include <memory_resource>
#include <span>

#include <catch2/catch_test_macros.hpp>

namespace luz::protocol::test
{
TEST_CASE("top row")
{
  constexpr auto top_row_p1
      = std::array{ std::byte{ 1 },   std::byte{ 52 },  std::byte{ 32 },  std::byte{ 2 },
                    std::byte{ 84 },  std::byte{ 17 },  std::byte{ 0 },   std::byte{ 224 },
                    std::byte{ 52 },  std::byte{ 0 },   std::byte{ 224 }, std::byte{ 87 },
                    std::byte{ 0 },   std::byte{ 227 }, std::byte{ 122 }, std::byte{ 0 },
                    std::byte{ 227 }, std::byte{ 157 }, std::byte{ 0 },   std::byte{ 227 } };
  constexpr auto top_row_p2
      = std::array{ std::byte{ 192 }, std::byte{ 0 },   std::byte{ 227 }, std::byte{ 227 },
                    std::byte{ 0 },   std::byte{ 227 }, std::byte{ 6 },   std::byte{ 1 },
                    std::byte{ 227 }, std::byte{ 41 },  std::byte{ 1 },   std::byte{ 227 },
                    std::byte{ 76 },  std::byte{ 1 },   std::byte{ 28 },  std::byte{ 111 },
                    std::byte{ 1 },   std::byte{ 3 },   std::byte{ 146 }, std::byte{ 1 } };
  constexpr auto top_row_p3
      = std::array{ std::byte{ 3 },   std::byte{ 181 }, std::byte{ 1 },  std::byte{ 3 },
                    std::byte{ 216 }, std::byte{ 1 },   std::byte{ 3 },  std::byte{ 251 },
                    std::byte{ 1 },   std::byte{ 227 }, std::byte{ 30 }, std::byte{ 2 },
                    std::byte{ 227 }, std::byte{ 65 },  std::byte{ 2 },  std::byte{ 227 },
                    std::byte{ 3 } };

  auto protocol = Protocol{};
  auto placement_allocator = protocol.placement_allocator();
  auto placements = placement_allocator.vector();

  REQUIRE_FALSE(protocol.process(top_row_p1, placements));
  REQUIRE_FALSE(protocol.process(top_row_p2, placements));
  REQUIRE(protocol.process(top_row_p3, placements));

  REQUIRE(placements.size() == 17UL);
  REQUIRE(placements[0].position == 17UL);

  for (size_t i = 1UL; i < placements.size(); ++i)
  {
    REQUIRE(placements[i].position - placements[i - 1UL].position == 35UL);
  }
}

TEST_CASE("decode wilbur_write_takes_flight")
{
  constexpr auto wilbur_wright_takes_flight_p1
      = std::array{ std::byte{ 0x01 }, std::byte{ 0x1F }, std::byte{ 0xD6 }, std::byte{ 0x02 },
                    std::byte{ 0x54 }, std::byte{ 0x29 }, std::byte{ 0x01 }, std::byte{ 0xE0 },
                    std::byte{ 0x6C }, std::byte{ 0x00 }, std::byte{ 0xE3 }, std::byte{ 0x8D },
                    std::byte{ 0x01 }, std::byte{ 0x03 }, std::byte{ 0x12 }, std::byte{ 0x01 },
                    std::byte{ 0x1C }, std::byte{ 0xAA }, std::byte{ 0x00 }, std::byte{ 0x1C } };
  constexpr auto wilbur_wright_takes_flight_p2 = std::array{
    std::byte{ 0xEC }, std::byte{ 0x00 }, std::byte{ 0x03 }, std::byte{ 0x0F },
    std::byte{ 0x01 }, std::byte{ 0x03 }, std::byte{ 0x34 }, std::byte{ 0x01 },
    std::byte{ 0xE3 }, std::byte{ 0x7C }, std::byte{ 0x01 }, std::byte{ 0xE3 },
    std::byte{ 0x78 }, std::byte{ 0x01 }, std::byte{ 0x03 }, std::byte{ 0x03 },
  };

  constexpr auto expected_placements = std::array{
    Placement{ 297U, Color{ 224, 0, 0 } },   Placement{ 108U, Color{ 224, 0, 192 } },
    Placement{ 397U, Color{ 0, 0, 192 } },   Placement{ 274U, Color{ 0, 224, 0 } },
    Placement{ 170U, Color{ 0, 224, 0 } },   Placement{ 236U, Color{ 0, 0, 192 } },
    Placement{ 271U, Color{ 0, 0, 192 } },   Placement{ 308U, Color{ 224, 0, 192 } },
    Placement{ 380U, Color{ 224, 0, 192 } }, Placement{ 376U, Color{ 0, 0, 192 } },
  };

  Protocol protocol{};
  auto placement_allocator = protocol.placement_allocator();
  auto placements = placement_allocator.vector();
  REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1, placements));
  REQUIRE(protocol.process(wilbur_wright_takes_flight_p2, placements));

  REQUIRE(placements.size() == expected_placements.size());

  for (size_t i = 0UL; i < expected_placements.size(); ++i)
  {
    REQUIRE(placements[i] == expected_placements[i]);
  }
}

// TODO ADD FAILURE TEST CASES

// Little Critters
// TEST_CASE("decode little_critters")
//{
//  // TODO verify payload
//  constexpr auto little_critters_p1 = std::array{
//    std::byte{ 0XCE }, std::byte{ 0X1 },  std::byte{ 0XE3 }, std::byte{ 0X6 }, std::byte{ 0X1 },
//    std::byte{ 0XE0 }, std::byte{ 0X57 }, std::byte{ 0X1 },  std::byte{ 0X3 }, std::byte{ 0XF },
//    std::byte{ 0X1 },  std::byte{ 0X3 },  std::byte{ 0X7C }, std::byte{ 0X1 }, std::byte{ 0X1C },
//    std::byte{ 0XC2 }, std::byte{ 0X1 },  std::byte{ 0X1C }, std::byte{ 0X3 },
//  };
//  constexpr auto expected_placements = std::array{
//    Placement{ 327U, Color{ 0, 0, 192 } }, Placement{ 366U, Color{ 0, 0, 192 } },
//    Placement{ 433U, Color{ 0, 0, 192 } }, Placement{ 245U, Color{ 224, 0, 192 } },
//    Placement{ 322U, Color{ 0, 0, 192 } }, Placement{ 462U, Color{ 224, 0, 192 } },
//    Placement{ 262U, Color{ 224, 0, 0 } }, Placement{ 343U, Color{ 0, 0, 192 } },
//    Placement{ 271U, Color{ 0, 0, 192 } }, Placement{ 380U, Color{ 0, 224, 0 } },
//    Placement{ 450U, Color{ 0, 224, 0 } },
//  };

//  Protocol protocol{};
//  auto placement_allocator = protocol.placement_allocator();
//  auto placements = placement_allocator.vector();

//  REQUIRE(protocol.process(little_critters_p1, placements));
//  REQUIRE(placements.size() == expected_placements.size());

//  for (size_t i = 0UL; i < expected_placements.size(); ++i)
//  {
//    REQUIRE(placements[i] == expected_placements[i]);
//  }
//}
} // namespace luz::protocol::test
