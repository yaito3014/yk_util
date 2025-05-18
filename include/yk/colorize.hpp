#ifndef YK_COLORIZE_HPP
#define YK_COLORIZE_HPP

#include <yk/detail/string_like.hpp>

#include <algorithm>
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

    if (color_ && style_) {
      return std::ranges::copy(
                 std::format("\033[{};{}m", std::to_underlying(*style_), std::to_underlying(*color_)), cc.out()
      )
          .out;
    } else if (color_) {
      return std::ranges::copy(std::format("\033[{}m", std::to_underlying(*color_)), cc.out()).out;
    } else {
      return std::ranges::copy(std::format("\033[{}m", std::to_underlying(*style_)), cc.out()).out;
    }
  }

private:
  enum class color : std::uint8_t {
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

  static constexpr color name_to_color(std::string_view name)
  {
    if (name == "black") return color::black;
    if (name == "red") return color::red;
    if (name == "green") return color::green;
    if (name == "yellow") return color::yellow;
    if (name == "blue") return color::blue;
    if (name == "magenta") return color::magenta;
    if (name == "cyan") return color::cyan;
    if (name == "white") return color::white;
    if (name == "bright_black") return color::bright_black;
    if (name == "bright_red") return color::bright_red;
    if (name == "bright_green") return color::bright_green;
    if (name == "bright_yellow") return color::bright_yellow;
    if (name == "bright_blue") return color::bright_blue;
    if (name == "bright_magenta") return color::bright_magenta;
    if (name == "bright_cyan") return color::bright_cyan;
    if (name == "bright_white") return color::bright_white;
    throw std::invalid_argument("invalid color name");
  }

  enum class style : std::uint8_t {
    normal = 0,
    bold = 1,
    italic = 3,
    underline = 4,
  };

  static constexpr style name_to_style(std::string_view name)
  {
    if (name == "normal") return style::normal;
    if (name == "bold") return style::bold;
    if (name == "italic") return style::italic;
    if (name == "underline") return style::underline;
    throw std::invalid_argument("invalid style name");
  }

  struct do_parse_result {
    typename basic_colorize_parse_context<CharT>::iterator in;
    std::optional<colorizer::color> color;
    std::optional<colorizer::style> style;
    bool reset = false;
  };

  static constexpr do_parse_result do_parse(basic_colorize_parse_context<CharT>& pc)
  {
    const auto starts_with = [](auto&& r1, auto&& r2) -> std::optional<std::ranges::iterator_t<decltype(r2)>> {
      auto [it1, it2] = std::ranges::mismatch(r1, r2);
      auto last2 = std::ranges::end(r2);
      if (it2 == last2) return it1;
      return std::nullopt;
    };

    using namespace std::string_view_literals;

    do_parse_result result;

    const auto assign_color = [&](std::string_view color_name, bool& modified) {
      if (auto opt = starts_with(pc, color_name)) {
        if (result.color) {
          throw colorize_error("multiple color cannot be specified");
        }
        result.in = *opt;
        result.color = name_to_color(color_name);
        modified = true;
      }
    };

    const auto assign_style = [&](std::string_view style_name, bool& modified) {
      if (auto opt = starts_with(pc, style_name)) {
        if (result.style) {
          throw colorize_error("multiple style cannot be specified");
        }
        result.in = *opt;
        result.style = name_to_style(style_name);
        modified = true;
      }
    };

    result.in = pc.begin();
    while (result.in != pc.end()) {
      if (*result.in == ']') break;
      if (*result.in == '|') pc.advance_to(++result.in);

      if (auto opt = starts_with(pc, "reset"sv)) {
        if (result.color || result.style) {
          throw colorize_error("none of other specifiers must be present when reset was specified");
        }
        if (**opt == ']') return {*opt, {}, {}, true};
        throw colorize_error("bracket mismatch");
      }

      bool modified = false;

      assign_color("black", modified);
      assign_color("red", modified);
      assign_color("green", modified);
      assign_color("yellow", modified);
      assign_color("blue", modified);
      assign_color("magenta", modified);
      assign_color("cyan", modified);
      assign_color("white", modified);
      assign_color("bright_black", modified);
      assign_color("bright_red", modified);
      assign_color("bright_green", modified);
      assign_color("bright_yellow", modified);
      assign_color("bright_blue", modified);
      assign_color("bright_magenta", modified);
      assign_color("bright_cyan", modified);
      assign_color("bright_white", modified);

      assign_style("normal", modified);
      assign_style("bold", modified);
      assign_style("italic", modified);
      assign_style("underline", modified);

      if (!modified) throw colorize_error("unknown specifier");
    }
    if (*result.in != ']') {
      throw colorize_error("unknown specifier");
    }

    if (!result.color && !result.style && !result.reset) {
      throw colorize_error("no specifier found");
    }

    return result;
  }

  std::optional<color> color_;
  std::optional<style> style_;
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
