#ifndef YK_COLORIZE_HPP
#define YK_COLORIZE_HPP

#include <yk/detail/string_like.hpp>

#include <algorithm>
#include <format>
#include <ranges>
#include <string_view>
#include <utility>

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

  explicit basic_colorize_context(Out out) : out_(std::move(out)) {}

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
    auto [it, color, style] = do_parse(pc);
    color_ = color;
    style_ = style;
    return it;
  }

  template <class Out>
  constexpr auto colorize(basic_colorize_context<Out, CharT>& cc) const
  {
    return std::ranges::copy(
               std::format("\033[{};{}m", std::to_underlying(style_), std::to_underlying(color_) + 30), cc.out()
    )
        .out;
  }

private:
  enum class color : std::uint8_t {
    black = 0,
    red = 1,
    green = 2,
    yellow = 3,
    blue = 4,
    magenta = 5,
    cyan = 6,
    white = 7,
  };

  enum class style : std::uint8_t {
    normal = 0,
    bold = 1,
    italic = 3,
    underline = 4,
  };

  struct do_parse_result {
    basic_colorize_parse_context<CharT>::iterator in;
    colorizer::color color;
    colorizer::style style;
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
    if (auto opt = starts_with(pc, "black"sv); opt && **opt == ']') return {*opt, color::black, style::normal};
    if (auto opt = starts_with(pc, "red"sv); opt && **opt == ']') return {*opt, color::red, style::normal};
    if (auto opt = starts_with(pc, "green"sv); opt && **opt == ']') return {*opt, color::green, style::normal};
    if (auto opt = starts_with(pc, "yellow"sv); opt && **opt == ']') return {*opt, color::yellow, style::normal};
    if (auto opt = starts_with(pc, "blue"sv); opt && **opt == ']') return {*opt, color::blue, style::normal};
    if (auto opt = starts_with(pc, "magenta"sv); opt && **opt == ']') return {*opt, color::magenta, style::normal};
    if (auto opt = starts_with(pc, "cyan"sv); opt && **opt == ']') return {*opt, color::cyan, style::normal};
    if (auto opt = starts_with(pc, "white"sv); opt && **opt == ']') return {*opt, color::white, style::normal};
    throw colorize_error("unkown color");
  }

  color color_;
  style style_;
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

  colorizing_scanner(basic_colorize_context<Out, CharT>& cc, std::basic_string_view<CharT> str)
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

template <class CharT>
struct basic_colorize_string {
  template <class S>
    requires detail::StringLike<const S&>
  consteval basic_colorize_string(const S& str) : str_(str)
  {
    detail::checking_scanner<CharT> scanner(str);
    scanner.scan();
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

  constexpr std::basic_string_view<CharT> get() const noexcept { return fmt_.get(); }

private:
  std::basic_format_string<CharT, Args...> fmt_;
};

template <class... Args>
using colorize_format_string = basic_colorize_format_string<char, std::type_identity_t<Args>...>;

template <class Out>
inline Out colorize_to(Out out, colorize_string col)
{
  basic_colorize_context<Out, char> ctx(std::move(out));
  detail::colorizing_scanner<Out, char> scanner(ctx, col.get());
  scanner.scan();
  return ctx.out();
}

inline std::string colorize(colorize_string col)
{
  std::string str;
  basic_colorize_context<std::back_insert_iterator<std::string>, char> ctx(std::back_inserter(str));
  detail::colorizing_scanner<std::back_insert_iterator<std::string>, char> scanner(ctx, col.get());
  scanner.scan();
  return str;
}

#if __cpp_lib_format >= 202311L

template <class Out, class... Args>
inline Out colorize_and_format_to(Out out, colorize_format_string<Args...> fmt, Args&&... args)
{
  return std::format_to(std::move(out), std::runtime_format(colorize(fmt)), std::forward<Args>(args)...);
}

template <class... Args>
inline std::string colorize_and_format(colorize_format_string<Args...> fmt, Args&&... args)
{
  return std::format(std::runtime_format(colorize(fmt)), std::forward<Args>(args)...);
}

#endif

}  // namespace yk

#endif  // YK_COLORIZE_HPP
