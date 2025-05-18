#ifndef YK_COLORIZE_HPP
#define YK_COLORIZE_HPP

#include "yk/detail/string_like.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <utility>
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

enum class color : std::uint8_t {
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

struct color_pair {
  std::string_view name;
  color value;
};

static constexpr auto color_lookup_table = [] {
  using namespace std::string_view_literals;
  std::array table{
      color_pair{"black", color::black},
      color_pair{"red", color::red},
      color_pair{"green", color::green},
      color_pair{"yellow", color::yellow},
      color_pair{"blue", color::blue},
      color_pair{"magenta", color::magenta},
      color_pair{"cyan", color::cyan},
      color_pair{"white", color::white},
      color_pair{"bright_black", color::bright_black},
      color_pair{"bright_red", color::bright_red},
      color_pair{"bright_green", color::bright_green},
      color_pair{"bright_yellow", color::bright_yellow},
      color_pair{"bright_blue", color::bright_blue},
      color_pair{"bright_magenta", color::bright_magenta},
      color_pair{"bright_cyan", color::bright_cyan},
      color_pair{"bright_white", color::bright_white},
  };
  std::ranges::sort(table, {}, &color_pair::name);
  return table;
}();

static constexpr color name_to_color(std::string_view name)
{
  auto it = std::ranges::lower_bound(color_lookup_table, name, {}, &color_pair::name);
  if (it == color_lookup_table.end() || it->name != name) return color::_empty;
  return it->value;
}

enum class style : std::uint8_t {
  normal = 0,
  bold = 1,
  italic = 3,
  underline = 4,
};

static constexpr std::optional<style> name_to_style(std::string_view name)
{
  if (name == "normal") return style::normal;
  if (name == "bold") return style::bold;
  if (name == "italic") return style::italic;
  if (name == "underline") return style::underline;
  return std::nullopt;
}

}  // namespace detail

template <class CharT = char>
struct colorizer {
  constexpr auto parse(basic_colorize_parse_context<CharT>& pc)
  {
    auto [it, color, style, reset] = do_parse(pc);
    color_ = color;
    style_ = style;
    reset_ = reset;
    return it;
  }

  template <class Out>
  constexpr auto colorize(basic_colorize_context<Out, CharT>& cc) const
  {
    if (reset_) return std::ranges::copy(std::string_view{"\033[0m"}, cc.out()).out;

    if (color_ != detail::color::_empty && style_ != detail::style::normal) {
      return std::ranges::copy(
                 std::format("\033[{};{}m", std::to_underlying(style_), std::to_underlying(color_)), cc.out()
      )
          .out;
    } else if (color_ != detail::color::_empty) {
      return std::ranges::copy(std::format("\033[{}m", std::to_underlying(color_)), cc.out()).out;
    } else {
      return std::ranges::copy(std::format("\033[{}m", std::to_underlying(style_)), cc.out()).out;
    }
  }

private:
  struct do_parse_result {
    typename basic_colorize_parse_context<CharT>::iterator in;
    detail::color color;
    detail::style style;
    bool reset;
  };

  static constexpr do_parse_result do_parse(basic_colorize_parse_context<CharT>& pc)
  {
    do_parse_result result{
        pc.begin(),
        detail::color::_empty,
        detail::style::normal,
        false,
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
      } else if (auto color = detail::name_to_color(specifier); color != detail::color::_empty) {
        if (result.color != detail::color::_empty) throw colorize_error("multiple color must not be specified");
        result.color = color;
      } else if (auto opt = detail::name_to_style(specifier)) {
        result.style = *opt;
      } else {
        throw colorize_error("invalid speicier");
      }

      pc.advance_to(it += len);
    }

    if (result.reset && (result.color != detail::color::_empty || result.style != detail::style::normal)) {
      throw colorize_error("reset must be independently specified");
    }

    return result;
  }

  detail::color color_;
  detail::style style_;
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
