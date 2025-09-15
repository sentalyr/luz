#include "protocol.hh"

#include <array>
#include <memory_resource>
#include <span>

#include <catch2/catch_test_macros.hpp>

namespace luz::protocol::test
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

constexpr auto wilbur_wright_takes_flight_expected = std::array{
  Placement{ 297U, Color{ 224, 0, 0 } },   Placement{ 108U, Color{ 224, 0, 192 } },
  Placement{ 397U, Color{ 0, 0, 192 } },   Placement{ 274U, Color{ 0, 224, 0 } },
  Placement{ 170U, Color{ 0, 224, 0 } },   Placement{ 236U, Color{ 0, 0, 192 } },
  Placement{ 271U, Color{ 0, 0, 192 } },   Placement{ 308U, Color{ 224, 0, 192 } },
  Placement{ 380U, Color{ 224, 0, 192 } }, Placement{ 376U, Color{ 0, 0, 192 } },
};

TEST_CASE("top row", "[top_row]")
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
  Packet packet{};

  REQUIRE_FALSE(protocol.process(top_row_p1, packet));
  REQUIRE_FALSE(protocol.process(top_row_p2, packet));
  REQUIRE(protocol.process(top_row_p3, packet));

  auto& placements = packet.placements;
  REQUIRE(placements.size() == 17UL);
  REQUIRE(placements[0].position == 17UL);

  for (size_t i = 1UL; i < placements.size(); ++i)
  {
    REQUIRE(placements[i].position - placements[i - 1UL].position == 35UL);
  }
}

TEST_CASE("decode wilbur_write_takes_flight")
{
  Protocol protocol{};
  Packet packet{};
  REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1, packet));
  REQUIRE(protocol.process(wilbur_wright_takes_flight_p2, packet));

  auto& placements = packet.placements;
  REQUIRE(placements.size() == wilbur_wright_takes_flight_expected.size());

  for (size_t i = 0UL; i < wilbur_wright_takes_flight_expected.size(); ++i)
  {
    REQUIRE(placements[i] == wilbur_wright_takes_flight_expected[i]);
  }
}

TEST_CASE("decode wilbur_write_takes_flight simulate dropped packet", "[packet_drop]")
{
  Protocol protocol{};
  Packet packet{};

  SECTION("2,1,2")
  {
    /// Start with part 2 and follow with parts 1 and 2
    REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p2, packet));
    REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1, packet));
    REQUIRE(protocol.process(wilbur_wright_takes_flight_p2, packet));
  }

  SECTION("1,1,2")
  {
    /// Start with part 1 and follow with parts 1 and 2
    REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1, packet));
    REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1, packet));
    REQUIRE(protocol.process(wilbur_wright_takes_flight_p2, packet));
  }

  auto& placements = packet.placements;
  REQUIRE(placements.size() == wilbur_wright_takes_flight_expected.size());

  for (size_t i = 0UL; i < wilbur_wright_takes_flight_expected.size(); ++i)
  {
    REQUIRE(placements[i] == wilbur_wright_takes_flight_expected[i]);
  }
}

TEST_CASE("decode wilbur_write_takes_flight incorrect byte indicators", "[byte_indicators]")
{
  Protocol protocol{};
  Packet packet{};

  auto wilbur_wright_takes_flight_p1_v
      = std::vector<std::byte>{ wilbur_wright_takes_flight_p1.begin(),
                                wilbur_wright_takes_flight_p1.end() };
  auto wilbur_wright_takes_flight_p2_v
      = std::vector<std::byte>{ wilbur_wright_takes_flight_p2.begin(),
                                wilbur_wright_takes_flight_p2.end() };

  SECTION("first byte indicator") { wilbur_wright_takes_flight_p1_v[0] = std::byte{ 0x02 }; }

  SECTION("second byte indicator") { wilbur_wright_takes_flight_p1_v[3] = std::byte{ 0x03 }; }

  SECTION("third byte indicator") { wilbur_wright_takes_flight_p2_v.back() = std::byte{ 0x01 }; }

  REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1_v, packet));
  REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p2_v, packet));
  REQUIRE_FALSE(protocol.process(wilbur_wright_takes_flight_p1, packet));
  REQUIRE(protocol.process(wilbur_wright_takes_flight_p2, packet));
}
} // namespace luz::protocol::test
