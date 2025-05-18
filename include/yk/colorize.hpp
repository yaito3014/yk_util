#ifndef YK_COLORIZE_HPP
#define YK_COLORIZE_HPP

#include "yk/detail/string_like.hpp"
#include "yk/enum_bitops.hpp"
#include "yk/enum_bitops_algorithm.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>
#include <version>

#include <cstdint>

namespace yk {

class colorize_error : public std::runtime_error {
  using runtime_error::runtime_error;
};

template <class CharT>
struct basic_colorize_parse_context {
  using char_type = CharT;
  using const_iterator = typename std::basic_string_view<CharT>::const_iterator;
  using iterator = const_iterator;

  constexpr explicit basic_colorize_parse_context(std::basic_string_view<CharT> fmt) noexcept
      : begin_(fmt.begin()), end_(fmt.end())
  {
  }

  constexpr iterator begin() const noexcept { return begin_; }
  constexpr iterator end() const noexcept { return end_; }
  constexpr void advance_to(iterator it) noexcept { begin_ = it; }

private:
  iterator begin_;
  iterator end_;
};

template <class Out, class CharT>
struct basic_colorize_context {
  using char_type = CharT;
  using iterator = Out;

  explicit constexpr basic_colorize_context(Out out) : out_(std::move(out)) {}

  constexpr iterator out() { return std::move(out_); }
  constexpr void advance_to(iterator it) { out_ = std::move(it); }

private:
  iterator out_;
};

using colorize_parse_context = basic_colorize_parse_context<char>;

namespace detail {

enum class ansi_color : std::uint8_t {
  _empty = 0,
  black = 30,
  red,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  white,
  bright_black = 90,
  bright_red,
  bright_green,
  bright_yellow,
  bright_blue,
  bright_magenta,
  bright_cyan,
  bright_white,
};

// https://github.com/fmtlib/fmt/blob/master/include/fmt/color.h
enum class rgb_color : uint32_t {
  alice_blue = 0xF0F8FF,               // rgb(240,248,255)
  antique_white = 0xFAEBD7,            // rgb(250,235,215)
  aqua = 0x00FFFF,                     // rgb(0,255,255)
  aquamarine = 0x7FFFD4,               // rgb(127,255,212)
  azure = 0xF0FFFF,                    // rgb(240,255,255)
  beige = 0xF5F5DC,                    // rgb(245,245,220)
  bisque = 0xFFE4C4,                   // rgb(255,228,196)
  black = 0x000000,                    // rgb(0,0,0)
  blanched_almond = 0xFFEBCD,          // rgb(255,235,205)
  blue = 0x0000FF,                     // rgb(0,0,255)
  blue_violet = 0x8A2BE2,              // rgb(138,43,226)
  brown = 0xA52A2A,                    // rgb(165,42,42)
  burly_wood = 0xDEB887,               // rgb(222,184,135)
  cadet_blue = 0x5F9EA0,               // rgb(95,158,160)
  chartreuse = 0x7FFF00,               // rgb(127,255,0)
  chocolate = 0xD2691E,                // rgb(210,105,30)
  coral = 0xFF7F50,                    // rgb(255,127,80)
  cornflower_blue = 0x6495ED,          // rgb(100,149,237)
  cornsilk = 0xFFF8DC,                 // rgb(255,248,220)
  crimson = 0xDC143C,                  // rgb(220,20,60)
  cyan = 0x00FFFF,                     // rgb(0,255,255)
  dark_blue = 0x00008B,                // rgb(0,0,139)
  dark_cyan = 0x008B8B,                // rgb(0,139,139)
  dark_golden_rod = 0xB8860B,          // rgb(184,134,11)
  dark_gray = 0xA9A9A9,                // rgb(169,169,169)
  dark_green = 0x006400,               // rgb(0,100,0)
  dark_khaki = 0xBDB76B,               // rgb(189,183,107)
  dark_magenta = 0x8B008B,             // rgb(139,0,139)
  dark_olive_green = 0x556B2F,         // rgb(85,107,47)
  dark_orange = 0xFF8C00,              // rgb(255,140,0)
  dark_orchid = 0x9932CC,              // rgb(153,50,204)
  dark_red = 0x8B0000,                 // rgb(139,0,0)
  dark_salmon = 0xE9967A,              // rgb(233,150,122)
  dark_sea_green = 0x8FBC8F,           // rgb(143,188,143)
  dark_slate_blue = 0x483D8B,          // rgb(72,61,139)
  dark_slate_gray = 0x2F4F4F,          // rgb(47,79,79)
  dark_turquoise = 0x00CED1,           // rgb(0,206,209)
  dark_violet = 0x9400D3,              // rgb(148,0,211)
  deep_pink = 0xFF1493,                // rgb(255,20,147)
  deep_sky_blue = 0x00BFFF,            // rgb(0,191,255)
  dim_gray = 0x696969,                 // rgb(105,105,105)
  dodger_blue = 0x1E90FF,              // rgb(30,144,255)
  fire_brick = 0xB22222,               // rgb(178,34,34)
  floral_white = 0xFFFAF0,             // rgb(255,250,240)
  forest_green = 0x228B22,             // rgb(34,139,34)
  fuchsia = 0xFF00FF,                  // rgb(255,0,255)
  gainsboro = 0xDCDCDC,                // rgb(220,220,220)
  ghost_white = 0xF8F8FF,              // rgb(248,248,255)
  gold = 0xFFD700,                     // rgb(255,215,0)
  golden_rod = 0xDAA520,               // rgb(218,165,32)
  gray = 0x808080,                     // rgb(128,128,128)
  green = 0x008000,                    // rgb(0,128,0)
  green_yellow = 0xADFF2F,             // rgb(173,255,47)
  honey_dew = 0xF0FFF0,                // rgb(240,255,240)
  hot_pink = 0xFF69B4,                 // rgb(255,105,180)
  indian_red = 0xCD5C5C,               // rgb(205,92,92)
  indigo = 0x4B0082,                   // rgb(75,0,130)
  ivory = 0xFFFFF0,                    // rgb(255,255,240)
  khaki = 0xF0E68C,                    // rgb(240,230,140)
  lavender = 0xE6E6FA,                 // rgb(230,230,250)
  lavender_blush = 0xFFF0F5,           // rgb(255,240,245)
  lawn_green = 0x7CFC00,               // rgb(124,252,0)
  lemon_chiffon = 0xFFFACD,            // rgb(255,250,205)
  light_blue = 0xADD8E6,               // rgb(173,216,230)
  light_coral = 0xF08080,              // rgb(240,128,128)
  light_cyan = 0xE0FFFF,               // rgb(224,255,255)
  light_golden_rod_yellow = 0xFAFAD2,  // rgb(250,250,210)
  light_gray = 0xD3D3D3,               // rgb(211,211,211)
  light_green = 0x90EE90,              // rgb(144,238,144)
  light_pink = 0xFFB6C1,               // rgb(255,182,193)
  light_salmon = 0xFFA07A,             // rgb(255,160,122)
  light_sea_green = 0x20B2AA,          // rgb(32,178,170)
  light_sky_blue = 0x87CEFA,           // rgb(135,206,250)
  light_slate_gray = 0x778899,         // rgb(119,136,153)
  light_steel_blue = 0xB0C4DE,         // rgb(176,196,222)
  light_yellow = 0xFFFFE0,             // rgb(255,255,224)
  lime = 0x00FF00,                     // rgb(0,255,0)
  lime_green = 0x32CD32,               // rgb(50,205,50)
  linen = 0xFAF0E6,                    // rgb(250,240,230)
  magenta = 0xFF00FF,                  // rgb(255,0,255)
  maroon = 0x800000,                   // rgb(128,0,0)
  medium_aquamarine = 0x66CDAA,        // rgb(102,205,170)
  medium_blue = 0x0000CD,              // rgb(0,0,205)
  medium_orchid = 0xBA55D3,            // rgb(186,85,211)
  medium_purple = 0x9370DB,            // rgb(147,112,219)
  medium_sea_green = 0x3CB371,         // rgb(60,179,113)
  medium_slate_blue = 0x7B68EE,        // rgb(123,104,238)
  medium_spring_green = 0x00FA9A,      // rgb(0,250,154)
  medium_turquoise = 0x48D1CC,         // rgb(72,209,204)
  medium_violet_red = 0xC71585,        // rgb(199,21,133)
  midnight_blue = 0x191970,            // rgb(25,25,112)
  mint_cream = 0xF5FFFA,               // rgb(245,255,250)
  misty_rose = 0xFFE4E1,               // rgb(255,228,225)
  moccasin = 0xFFE4B5,                 // rgb(255,228,181)
  navajo_white = 0xFFDEAD,             // rgb(255,222,173)
  navy = 0x000080,                     // rgb(0,0,128)
  old_lace = 0xFDF5E6,                 // rgb(253,245,230)
  olive = 0x808000,                    // rgb(128,128,0)
  olive_drab = 0x6B8E23,               // rgb(107,142,35)
  orange = 0xFFA500,                   // rgb(255,165,0)
  orange_red = 0xFF4500,               // rgb(255,69,0)
  orchid = 0xDA70D6,                   // rgb(218,112,214)
  pale_golden_rod = 0xEEE8AA,          // rgb(238,232,170)
  pale_green = 0x98FB98,               // rgb(152,251,152)
  pale_turquoise = 0xAFEEEE,           // rgb(175,238,238)
  pale_violet_red = 0xDB7093,          // rgb(219,112,147)
  papaya_whip = 0xFFEFD5,              // rgb(255,239,213)
  peach_puff = 0xFFDAB9,               // rgb(255,218,185)
  peru = 0xCD853F,                     // rgb(205,133,63)
  pink = 0xFFC0CB,                     // rgb(255,192,203)
  plum = 0xDDA0DD,                     // rgb(221,160,221)
  powder_blue = 0xB0E0E6,              // rgb(176,224,230)
  purple = 0x800080,                   // rgb(128,0,128)
  rebecca_purple = 0x663399,           // rgb(102,51,153)
  red = 0xFF0000,                      // rgb(255,0,0)
  rosy_brown = 0xBC8F8F,               // rgb(188,143,143)
  royal_blue = 0x4169E1,               // rgb(65,105,225)
  saddle_brown = 0x8B4513,             // rgb(139,69,19)
  salmon = 0xFA8072,                   // rgb(250,128,114)
  sandy_brown = 0xF4A460,              // rgb(244,164,96)
  sea_green = 0x2E8B57,                // rgb(46,139,87)
  sea_shell = 0xFFF5EE,                // rgb(255,245,238)
  sienna = 0xA0522D,                   // rgb(160,82,45)
  silver = 0xC0C0C0,                   // rgb(192,192,192)
  sky_blue = 0x87CEEB,                 // rgb(135,206,235)
  slate_blue = 0x6A5ACD,               // rgb(106,90,205)
  slate_gray = 0x708090,               // rgb(112,128,144)
  snow = 0xFFFAFA,                     // rgb(255,250,250)
  spring_green = 0x00FF7F,             // rgb(0,255,127)
  steel_blue = 0x4682B4,               // rgb(70,130,180)
  tan = 0xD2B48C,                      // rgb(210,180,140)
  teal = 0x008080,                     // rgb(0,128,128)
  thistle = 0xD8BFD8,                  // rgb(216,191,216)
  tomato = 0xFF6347,                   // rgb(255,99,71)
  turquoise = 0x40E0D0,                // rgb(64,224,208)
  violet = 0xEE82EE,                   // rgb(238,130,238)
  wheat = 0xF5DEB3,                    // rgb(245,222,179)
  white = 0xFFFFFF,                    // rgb(255,255,255)
  white_smoke = 0xF5F5F5,              // rgb(245,245,245)
  yellow = 0xFFFF00,                   // rgb(255,255,0)
  yellow_green = 0x9ACD32              // rgb(154,205,50)
};

constexpr auto get_rgb(rgb_color rgb)
{
  std::uint32_t val = std::to_underlying(rgb);
  return std::make_tuple((val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
}

class color {
public:
  constexpr color() : value() {}
  constexpr color(ansi_color ansi) : value(ansi) {}
  constexpr color(rgb_color rgb) : value(rgb) {}

  constexpr bool is_ansi_color() const { return std::holds_alternative<ansi_color>(value); }
  constexpr bool is_rgb_color() const { return std::holds_alternative<rgb_color>(value); }

  constexpr ansi_color get_ansi_color() const { return std::get<ansi_color>(value); }
  constexpr rgb_color get_rgb_color() const { return std::get<rgb_color>(value); }

  constexpr bool empty() const { return std::holds_alternative<std::monostate>(value); }

private:
  std::variant<std::monostate, ansi_color, rgb_color> value;
};

struct color_pair {
  std::string_view name;
  color value;
};

static constexpr auto color_lookup_table = [] {
  using namespace std::string_view_literals;
  std::array table{
      color_pair{"black", ansi_color::black},
      color_pair{"red", ansi_color::red},
      color_pair{"green", ansi_color::green},
      color_pair{"yellow", ansi_color::yellow},
      color_pair{"blue", ansi_color::blue},
      color_pair{"magenta", ansi_color::magenta},
      color_pair{"cyan", ansi_color::cyan},
      color_pair{"white", ansi_color::white},
      color_pair{"bright_black", ansi_color::bright_black},
      color_pair{"bright_red", ansi_color::bright_red},
      color_pair{"bright_green", ansi_color::bright_green},
      color_pair{"bright_yellow", ansi_color::bright_yellow},
      color_pair{"bright_blue", ansi_color::bright_blue},
      color_pair{"bright_magenta", ansi_color::bright_magenta},
      color_pair{"bright_cyan", ansi_color::bright_cyan},
      color_pair{"bright_white", ansi_color::bright_white},
      color_pair{"alice_blue", rgb_color::alice_blue},
      color_pair{"antique_white", rgb_color::antique_white},
      color_pair{"aqua", rgb_color::aqua},
      color_pair{"aquamarine", rgb_color::aquamarine},
      color_pair{"azure", rgb_color::azure},
      color_pair{"beige", rgb_color::beige},
      color_pair{"bisque", rgb_color::bisque},
      color_pair{"rgb_black", rgb_color::black},
      color_pair{"blanched_almond", rgb_color::blanched_almond},
      color_pair{"rgb_blue", rgb_color::blue},
      color_pair{"blue_violet", rgb_color::blue_violet},
      color_pair{"brown", rgb_color::brown},
      color_pair{"burly_wood", rgb_color::burly_wood},
      color_pair{"cadet_blue", rgb_color::cadet_blue},
      color_pair{"chartreuse", rgb_color::chartreuse},
      color_pair{"chocolate", rgb_color::chocolate},
      color_pair{"coral", rgb_color::coral},
      color_pair{"cornflower_blue", rgb_color::cornflower_blue},
      color_pair{"cornsilk", rgb_color::cornsilk},
      color_pair{"crimson", rgb_color::crimson},
      color_pair{"rgb_cyan", rgb_color::cyan},
      color_pair{"dark_blue", rgb_color::dark_blue},
      color_pair{"dark_cyan", rgb_color::dark_cyan},
      color_pair{"dark_golden_rod", rgb_color::dark_golden_rod},
      color_pair{"dark_gray", rgb_color::dark_gray},
      color_pair{"dark_green", rgb_color::dark_green},
      color_pair{"dark_khaki", rgb_color::dark_khaki},
      color_pair{"dark_magenta", rgb_color::dark_magenta},
      color_pair{"dark_olive_green", rgb_color::dark_olive_green},
      color_pair{"dark_orange", rgb_color::dark_orange},
      color_pair{"dark_orchid", rgb_color::dark_orchid},
      color_pair{"dark_red", rgb_color::dark_red},
      color_pair{"dark_salmon", rgb_color::dark_salmon},
      color_pair{"dark_sea_green", rgb_color::dark_sea_green},
      color_pair{"dark_slate_blue", rgb_color::dark_slate_blue},
      color_pair{"dark_slate_gray", rgb_color::dark_slate_gray},
      color_pair{"dark_turquoise", rgb_color::dark_turquoise},
      color_pair{"dark_violet", rgb_color::dark_violet},
      color_pair{"deep_pink", rgb_color::deep_pink},
      color_pair{"deep_sky_blue", rgb_color::deep_sky_blue},
      color_pair{"dim_gray", rgb_color::dim_gray},
      color_pair{"dodger_blue", rgb_color::dodger_blue},
      color_pair{"fire_brick", rgb_color::fire_brick},
      color_pair{"floral_white", rgb_color::floral_white},
      color_pair{"forest_green", rgb_color::forest_green},
      color_pair{"fuchsia", rgb_color::fuchsia},
      color_pair{"gainsboro", rgb_color::gainsboro},
      color_pair{"ghost_white", rgb_color::ghost_white},
      color_pair{"gold", rgb_color::gold},
      color_pair{"golden_rod", rgb_color::golden_rod},
      color_pair{"gray", rgb_color::gray},
      color_pair{"rgb_green", rgb_color::green},
      color_pair{"green_yellow", rgb_color::green_yellow},
      color_pair{"honey_dew", rgb_color::honey_dew},
      color_pair{"hot_pink", rgb_color::hot_pink},
      color_pair{"indian_red", rgb_color::indian_red},
      color_pair{"indigo", rgb_color::indigo},
      color_pair{"ivory", rgb_color::ivory},
      color_pair{"khaki", rgb_color::khaki},
      color_pair{"lavender", rgb_color::lavender},
      color_pair{"lavender_blush", rgb_color::lavender_blush},
      color_pair{"lawn_green", rgb_color::lawn_green},
      color_pair{"lemon_chiffon", rgb_color::lemon_chiffon},
      color_pair{"light_blue", rgb_color::light_blue},
      color_pair{"light_coral", rgb_color::light_coral},
      color_pair{"light_cyan", rgb_color::light_cyan},
      color_pair{"light_golden_rod_yellow", rgb_color::light_golden_rod_yellow},
      color_pair{"light_gray", rgb_color::light_gray},
      color_pair{"light_green", rgb_color::light_green},
      color_pair{"light_pink", rgb_color::light_pink},
      color_pair{"light_salmon", rgb_color::light_salmon},
      color_pair{"light_sea_green", rgb_color::light_sea_green},
      color_pair{"light_sky_blue", rgb_color::light_sky_blue},
      color_pair{"light_slate_gray", rgb_color::light_slate_gray},
      color_pair{"light_steel_blue", rgb_color::light_steel_blue},
      color_pair{"light_yellow", rgb_color::light_yellow},
      color_pair{"lime", rgb_color::lime},
      color_pair{"lime_green", rgb_color::lime_green},
      color_pair{"linen", rgb_color::linen},
      color_pair{"rgb_magenta", rgb_color::magenta},
      color_pair{"maroon", rgb_color::maroon},
      color_pair{"medium_aquamarine", rgb_color::medium_aquamarine},
      color_pair{"medium_blue", rgb_color::medium_blue},
      color_pair{"medium_orchid", rgb_color::medium_orchid},
      color_pair{"medium_purple", rgb_color::medium_purple},
      color_pair{"medium_sea_green", rgb_color::medium_sea_green},
      color_pair{"medium_slate_blue", rgb_color::medium_slate_blue},
      color_pair{"medium_spring_green", rgb_color::medium_spring_green},
      color_pair{"medium_turquoise", rgb_color::medium_turquoise},
      color_pair{"medium_violet_red", rgb_color::medium_violet_red},
      color_pair{"midnight_blue", rgb_color::midnight_blue},
      color_pair{"mint_cream", rgb_color::mint_cream},
      color_pair{"misty_rose", rgb_color::misty_rose},
      color_pair{"moccasin", rgb_color::moccasin},
      color_pair{"navajo_white", rgb_color::navajo_white},
      color_pair{"navy", rgb_color::navy},
      color_pair{"old_lace", rgb_color::old_lace},
      color_pair{"olive", rgb_color::olive},
      color_pair{"olive_drab", rgb_color::olive_drab},
      color_pair{"orange", rgb_color::orange},
      color_pair{"orange_red", rgb_color::orange_red},
      color_pair{"orchid", rgb_color::orchid},
      color_pair{"pale_golden_rod", rgb_color::pale_golden_rod},
      color_pair{"pale_green", rgb_color::pale_green},
      color_pair{"pale_turquoise", rgb_color::pale_turquoise},
      color_pair{"pale_violet_red", rgb_color::pale_violet_red},
      color_pair{"papaya_whip", rgb_color::papaya_whip},
      color_pair{"peach_puff", rgb_color::peach_puff},
      color_pair{"peru", rgb_color::peru},
      color_pair{"pink", rgb_color::pink},
      color_pair{"plum", rgb_color::plum},
      color_pair{"powder_blue", rgb_color::powder_blue},
      color_pair{"purple", rgb_color::purple},
      color_pair{"rebecca_purple", rgb_color::rebecca_purple},
      color_pair{"rgb_red", rgb_color::red},
      color_pair{"rosy_brown", rgb_color::rosy_brown},
      color_pair{"royal_blue", rgb_color::royal_blue},
      color_pair{"saddle_brown", rgb_color::saddle_brown},
      color_pair{"salmon", rgb_color::salmon},
      color_pair{"sandy_brown", rgb_color::sandy_brown},
      color_pair{"sea_green", rgb_color::sea_green},
      color_pair{"sea_shell", rgb_color::sea_shell},
      color_pair{"sienna", rgb_color::sienna},
      color_pair{"silver", rgb_color::silver},
      color_pair{"sky_blue", rgb_color::sky_blue},
      color_pair{"slate_blue", rgb_color::slate_blue},
      color_pair{"slate_gray", rgb_color::slate_gray},
      color_pair{"snow", rgb_color::snow},
      color_pair{"spring_green", rgb_color::spring_green},
      color_pair{"steel_blue", rgb_color::steel_blue},
      color_pair{"tan", rgb_color::tan},
      color_pair{"teal", rgb_color::teal},
      color_pair{"thistle", rgb_color::thistle},
      color_pair{"tomato", rgb_color::tomato},
      color_pair{"turquoise", rgb_color::turquoise},
      color_pair{"violet", rgb_color::violet},
      color_pair{"wheat", rgb_color::wheat},
      color_pair{"rgb_white", rgb_color::white},
      color_pair{"white_smoke", rgb_color::white_smoke},
      color_pair{"rgb_yellow", rgb_color::yellow},
      color_pair{"yellow_green", rgb_color::yellow_green},
  };
  std::ranges::sort(table, {}, &color_pair::name);
  return table;
}();

static constexpr color name_to_color(std::string_view name)
{
  auto it = std::ranges::lower_bound(color_lookup_table, name, {}, &color_pair::name);
  if (it == color_lookup_table.end() || it->name != name) return color{};
  return it->value;
}

enum class emphasis : uint8_t {
  _empty = 0,
  bold = 1 << 0,
  faint = 1 << 1,
  italic = 1 << 2,
  underline = 1 << 3,
  blink = 1 << 4,
  reverse = 1 << 5,
  conceal = 1 << 6,
  strike = 1 << 7,
};

static constexpr emphasis name_to_emphasis(std::string_view name)
{
  if (name == "bold") return emphasis::bold;
  if (name == "faint") return emphasis::faint;
  if (name == "italic") return emphasis::italic;
  if (name == "underline") return emphasis::underline;
  if (name == "blink") return emphasis::blink;
  if (name == "reverse") return emphasis::reverse;
  if (name == "conceal") return emphasis::conceal;
  if (name == "strike") return emphasis::strike;
  return emphasis::_empty;
}

static constexpr std::uint8_t emphasis_to_value(emphasis em)
{
  switch (em) {
    case emphasis::bold:
      return 1;
    case emphasis::faint:
      return 2;
    case emphasis::italic:
      return 3;
    case emphasis::underline:
      return 4;
    case emphasis::blink:
      return 5;
    case emphasis::reverse:
      return 7;
    case emphasis::conceal:
      return 8;
    case emphasis::strike:
      return 9;
    default:
      throw std::invalid_argument("invalid emphasis");
  }
}

}  // namespace detail

template <>
struct bitops_enabled<detail::emphasis> : std::true_type {
  static constexpr int max_bit = 7;
};

template <class CharT = char>
struct colorizer {
  constexpr auto parse(basic_colorize_parse_context<CharT>& pc)
  {
    auto [it, fg_color, bg_color, emphasis, reset] = do_parse(pc);
    fg_color_ = fg_color;
    bg_color_ = bg_color;
    emphasis_ = emphasis;
    reset_ = reset;
    return it;
  }

  template <class Out>
  constexpr auto colorize(basic_colorize_context<Out, CharT>& cc) const
  {
    using namespace std::string_view_literals;

    // shorhand for reset
    if (reset_) return std::ranges::copy("\033[0m"sv, cc.out()).out;

    // write prefix
    auto it = std::ranges::copy("\033["sv, cc.out()).out;

    bool first = true;

    // write emphases
    for (auto em : each_bit(emphasis_)) {
      if (!first) *it++ = ';';
      first = false;
      it = std::format_to(it, "{}", emphasis_to_value(em));
    }

    // write foreground color
    if (!fg_color_.empty()) {
      if (!first) *it++ = ';';
      first = false;
      if (fg_color_.is_ansi_color()) {
        it = std::format_to(it, "{}", std::to_underlying(fg_color_.get_ansi_color()));
      } else {
        detail::rgb_color rgb = fg_color_.get_rgb_color();
        auto [r, g, b] = detail::get_rgb(rgb);
        it = std::format_to(it, "38;5;{};{};{}", r, g, b);
      }
    }

    // write background color
    if (!bg_color_.empty()) {
      if (!first) *it++ = ';';
      first = false;
      if (bg_color_.is_ansi_color()) {
        it = std::format_to(it, "{}", std::to_underlying(bg_color_.get_ansi_color()) + 10);
      } else {
        detail::rgb_color rgb = bg_color_.get_rgb_color();
        auto [r, g, b] = detail::get_rgb(rgb);
        it = std::format_to(it, "48;5;{};{};{}", r, g, b);
      }
    }

    // write suffix
    *it++ = 'm';

    return it;
  }

private:
  struct do_parse_result {
    typename basic_colorize_parse_context<CharT>::iterator in;
    detail::color fg_color;
    detail::color bg_color;
    detail::emphasis emphasis;
    bool reset;
  };

  static constexpr do_parse_result do_parse(basic_colorize_parse_context<CharT>& pc)
  {
    do_parse_result result{
        pc.begin(), detail::color{}, detail::color{}, detail::emphasis::_empty, false,
    };

    auto& it = result.in;
    while (it != pc.end()) {
      if (*it == ']') break;
      if (*it == '|') pc.advance_to(++it);

      const std::string_view rest{it, pc.end()};
      const auto len = rest.find_first_of("|]");
      const std::string_view specifier = rest.substr(0, len);

      if (specifier == "reset") {
        result.reset = true;
      } else if (auto fg_color = detail::name_to_color(specifier); !fg_color.empty()) {
        if (!result.fg_color.empty()) throw colorize_error("multiple color must not be specified");
        result.fg_color = fg_color;
      } else if (auto emphasis = detail::name_to_emphasis(specifier); emphasis != detail::emphasis::_empty) {
        using namespace bitops_operators;
        result.emphasis |= emphasis;
      } else if (specifier.starts_with("fg:")) {
        if (auto fg_color = detail::name_to_color(specifier.substr(3)); !fg_color.empty()) {
          if (!result.fg_color.empty()) throw colorize_error("multiple color must not be specified");
          result.fg_color = fg_color;
        } else {
          throw colorize_error("fg prefix must precedes color name");
        }
      } else if (specifier.starts_with("bg:")) {
        if (auto bg_color = detail::name_to_color(specifier.substr(3)); !bg_color.empty()) {
          if (!result.bg_color.empty()) throw colorize_error("multiple color must not be specified");
          result.bg_color = bg_color;
        } else {
          throw colorize_error("bg prefix must precedes color name");
        }
      } else {
        throw colorize_error("invalid speicier");
      }

      pc.advance_to(it += len);
    }

    if (result.reset && (!result.fg_color.empty() || result.emphasis != detail::emphasis::_empty)) {
      throw colorize_error("reset must be independently specified");
    }

    return result;
  }

  detail::color fg_color_;
  detail::color bg_color_;
  detail::emphasis emphasis_;
  bool reset_;
};

namespace detail {

template <class CharT>
struct scanner {
  using iterator = typename basic_colorize_parse_context<CharT>::iterator;

  constexpr scanner(std::basic_string_view<CharT> str) noexcept : pc(str) {}

  constexpr void scan()
  {
    std::basic_string_view<CharT> fmt{pc.begin(), pc.end()};
    auto left_bracket = fmt.find('[');
    auto right_bracket = fmt.find(']');
    while (fmt.size()) {
      auto comp = left_bracket <=> right_bracket;
      if (comp == 0) {
        // no brackets found, consuming rest
        on_chars(pc.end());
        pc.advance_to(pc.end());
        return;
      } else if (comp < 0) {
        // normal case
        if (left_bracket + 1 == fmt.size() || (right_bracket == fmt.npos && fmt[left_bracket + 1] == '[')) {
          throw colorize_error("unmatched left bracket");
        }
        const bool is_escape = fmt[left_bracket + 1] == '[';
        const auto last = pc.begin() + left_bracket + is_escape;
        on_chars(last);
        pc.advance_to(last + 1);
        fmt = {pc.begin(), pc.end()};
        if (is_escape) {
          if (right_bracket != fmt.npos) right_bracket -= left_bracket + 2;
          left_bracket = fmt.find('[');
        } else {
          on_replacement_field();
          fmt = {pc.begin(), pc.end()};
          left_bracket = fmt.find('[');
          right_bracket = fmt.find(']');
        }
      } else {
        if (++right_bracket == fmt.size() || fmt[right_bracket] != ']') {
          throw colorize_error("unmatched right bracket");
        }
        auto last = pc.begin() + right_bracket;
        on_chars(last);
        pc.advance_to(last + 1);
        fmt = {pc.begin(), pc.end()};
        if (left_bracket != fmt.npos) left_bracket -= right_bracket + 1;
        right_bracket = fmt.find(']');
      }
    }
  }

  constexpr void on_replacement_field()
  {
    auto next = pc.begin();
    if (*next == ']') {
      throw colorize_error("empty color specifier");
    }
    colorize();
    if (pc.begin() == pc.end() || *pc.begin() != ']') {
      throw colorize_error("unmatched left brace");
    }
    pc.advance_to(pc.begin() + 1);
  }

  constexpr virtual void on_chars(iterator) {}
  constexpr virtual void colorize() = 0;

  basic_colorize_parse_context<CharT> pc;
};

template <class CharT>
struct checking_scanner : scanner<CharT> {
  consteval checking_scanner(std::basic_string_view<CharT> str) : scanner<CharT>(str) {}

  constexpr void colorize() override { parse_colorize_spec(); }

  constexpr void parse_colorize_spec()
  {
    colorizer<CharT> c;
    this->pc.advance_to(c.parse(this->pc));
  }
};

template <class Out, class CharT>
struct colorizing_scanner : scanner<CharT> {
  using iterator = typename scanner<CharT>::iterator;

  constexpr colorizing_scanner(basic_colorize_context<Out, CharT>& cc, std::basic_string_view<CharT> str)
      : scanner<CharT>(str), cc(cc)
  {
  }

  constexpr void on_chars(iterator last) override
  {
    std::basic_string_view<CharT> str(this->pc.begin(), last);
    cc.advance_to(std::ranges::copy(str, cc.out()).out);
  }

  constexpr void colorize() override
  {
    colorizer c;
    this->pc.advance_to(c.parse(this->pc));
    cc.advance_to(c.colorize(cc));
  }

  basic_colorize_context<Out, CharT>& cc;
};

}  // namespace detail

namespace detail {

template <class CharT>
struct basic_runtime_colorize_string {
  constexpr explicit basic_runtime_colorize_string(std::basic_string_view<CharT> str) : str_(str) {}
  std::basic_string_view<CharT> str_;
};

template <class CharT>
struct basic_runtime_colorize_format_string {
  constexpr explicit basic_runtime_colorize_format_string(std::basic_string_view<CharT> str) : str_(str) {}
  std::basic_string_view<CharT> str_;
};

}  // namespace detail

constexpr auto runtime_colorize(std::string_view str) { return detail::basic_runtime_colorize_string<char>{str}; }
constexpr auto runtime_colorize_format(std::string_view str)
{
  return detail::basic_runtime_colorize_format_string<char>{str};
}

template <class CharT>
struct basic_colorize_string {
  template <class S>
    requires detail::StringLike<const S&>
  consteval basic_colorize_string(const S& str) : str_(str)
  {
    detail::checking_scanner<CharT> scanner(str);
    scanner.scan();
  }

  constexpr basic_colorize_string(detail::basic_runtime_colorize_string<CharT> runtime_str) noexcept
      : str_(runtime_str.str_)
  {
  }

  constexpr std::basic_string_view<CharT> get() const noexcept { return str_; }

private:
  std::basic_string_view<CharT> str_;
};

using colorize_string = basic_colorize_string<char>;

template <class CharT, class... Args>
struct basic_colorize_format_string {
  template <class S>
    requires detail::StringLike<const S&>
  consteval basic_colorize_format_string(const S& str) : fmt_(str)
  {
    detail::checking_scanner<CharT> scanner(str);
    scanner.scan();
  }

#if __cpp_lib_format >= 202411L
  explicit constexpr basic_colorize_format_string(detail::basic_runtime_colorize_format_string<CharT> runtime_str)
      : fmt_(std::runtime_format(runtime_str.str_))
  {
  }
#endif

  constexpr std::basic_format_string<CharT, Args...> get() const noexcept { return fmt_; }

private:
  std::basic_format_string<CharT, Args...> fmt_;
};

template <class... Args>
using colorize_format_string = basic_colorize_format_string<char, std::type_identity_t<Args>...>;

template <class Out>
inline constexpr Out colorize_to(Out out, colorize_string col)
{
  basic_colorize_context<Out, char> ctx(std::move(out));
  detail::colorizing_scanner<Out, char> scanner(ctx, col.get());
  scanner.scan();
  return ctx.out();
}

inline constexpr std::string colorize(colorize_string col)
{
  std::string str;
  basic_colorize_context<std::back_insert_iterator<std::string>, char> ctx(std::back_inserter(str));
  detail::colorizing_scanner<std::back_insert_iterator<std::string>, char> scanner(ctx, col.get());
  scanner.scan();
  return str;
}

template <class Out, class... Args>
inline constexpr Out format_and_colorize_to(Out out, colorize_format_string<Args...> fmt, Args&&... args)
{
  return colorize_to(std::move(out), runtime_colorize(std::format(fmt.get(), std::forward<Args>(args)...)));
}

template <class... Args>
inline constexpr std::string format_and_colorize(colorize_format_string<Args...> fmt, Args&&... args)
{
  return colorize(runtime_colorize(std::format(fmt.get(), std::forward<Args>(args)...)));
}

}  // namespace yk

#endif  // YK_COLORIZE_HPP
