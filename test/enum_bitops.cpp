#include "yk/enum_bitops.hpp"
#include "yk/enum_bitops_algorithm.hpp"
#include "yk/enum_bitops_io.hpp"

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

enum class MyFlags : std::uint8_t {
  FOO = 1 << 0,
  BAR = 1 << 1,
  BAZ = 1 << 2,
};

enum class SpellType : std::uint8_t {
  TYPE_ATTACK = 1 << 0,
  TYPE_DEFENSE = 1 << 1,

  ATTR_FIRE = 1 << 2,
  ATTR_WATER = 1 << 3,
  ATTR_THUNDER = 1 << 4,
};

template <>
struct yk::bitops_enabled<MyFlags> : std::true_type {
  static MyFlags parse(std::string_view sv) noexcept {
    using enum MyFlags;
    if (sv == "foo") return FOO;
    if (sv == "bar") return BAR;
    if (sv == "baz") return BAZ;
    return {};
  }
};

template <>
struct yk::bitops_enabled<SpellType> : std::true_type {
  static constexpr int min_bit = 2;
  static constexpr int max_bit = 4;
};

BOOST_AUTO_TEST_SUITE(EnumBitops)

BOOST_AUTO_TEST_CASE(Enum) {
  using namespace yk::bitops_operators;

  using enum MyFlags;

  BOOST_TEST((~FOO == static_cast<MyFlags>(~yk::to_underlying(FOO))));

  BOOST_TEST(((FOO & BAR) == static_cast<MyFlags>(yk::to_underlying(FOO) & yk::to_underlying(BAR))));
  BOOST_TEST(((FOO ^ BAR) == static_cast<MyFlags>(yk::to_underlying(FOO) ^ yk::to_underlying(BAR))));
  BOOST_TEST(((FOO | BAR) == static_cast<MyFlags>(yk::to_underlying(FOO) | yk::to_underlying(BAR))));

  // clang-format off

  BOOST_TEST( yk::contains(FOO, FOO));
  BOOST_TEST(!yk::contains(FOO, BAR));
  BOOST_TEST( yk::contains(FOO | BAR, FOO));
  BOOST_TEST(!yk::contains(FOO | BAR, BAZ));
  BOOST_TEST(!yk::contains(FOO |       BAZ, FOO | BAR));
  BOOST_TEST(!yk::contains(      BAR | BAZ, FOO | BAR));
  BOOST_TEST( yk::contains(FOO | BAR | BAZ, FOO | BAR));

  BOOST_TEST( yk::contains_any_bit(FOO, FOO));
  BOOST_TEST(!yk::contains_any_bit(FOO, BAR));
  BOOST_TEST( yk::contains_any_bit(FOO | BAR, FOO));
  BOOST_TEST(!yk::contains_any_bit(FOO | BAR, BAZ));
  BOOST_TEST( yk::contains_any_bit(FOO |       BAZ, FOO | BAR));
  BOOST_TEST( yk::contains_any_bit(      BAR | BAZ, FOO | BAR));
  BOOST_TEST( yk::contains_any_bit(FOO | BAR | BAZ, FOO | BAR));

  BOOST_TEST( yk::contains_single_bit(FOO, FOO));
  BOOST_TEST(!yk::contains_single_bit(FOO, BAR));
  BOOST_TEST( yk::contains_single_bit(FOO | BAR, FOO));
  BOOST_TEST(!yk::contains_single_bit(FOO | BAR, BAZ));

  // clang-format on

  BOOST_TEST((yk::parse_flag<MyFlags>("foo") == FOO));
  BOOST_TEST((yk::parse_flag<MyFlags>("bar") == BAR));
  BOOST_TEST((yk::parse_flag<MyFlags>("baz") == BAZ));
  BOOST_TEST((yk::parse_flag<MyFlags>("yay") == MyFlags{}));

  BOOST_TEST((yk::parse_flags<MyFlags>("foo", "|") == FOO));
  BOOST_TEST((yk::parse_flags<MyFlags>("yay", "|") == MyFlags{}));

  BOOST_TEST((yk::parse_flags<MyFlags>("foo|bar", "|") == (FOO | BAR)));
  BOOST_TEST((yk::parse_flags<MyFlags>("foo|yay", "|") == MyFlags{}));
  BOOST_TEST((yk::parse_flags<MyFlags>("foo,bar", "|") == MyFlags{}));

  BOOST_TEST(std::ranges::equal(yk::each_bit(SpellType::TYPE_ATTACK | SpellType::ATTR_FIRE | SpellType::ATTR_THUNDER),
                                std::vector{SpellType::ATTR_FIRE, SpellType::ATTR_THUNDER}));

  BOOST_TEST(std::ranges::equal(yk::each_bit(static_cast<SpellType>(static_cast<std::underlying_type_t<SpellType>>(-1))),
                                std::vector{SpellType::ATTR_FIRE, SpellType::ATTR_WATER, SpellType::ATTR_THUNDER}));
}

BOOST_AUTO_TEST_SUITE_END()
