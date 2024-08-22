#ifndef YK_UTIL_TO_SUBRANGE_STD_HPP
#define YK_UTIL_TO_SUBRANGE_STD_HPP

#include <ranges>
#include <utility>

namespace yk {

template <class Iterator>
[[nodiscard]] constexpr auto to_subrange(const std::pair<Iterator, Iterator>& t) noexcept(noexcept(std::ranges::subrange(t.first, t.second)))
    -> std::ranges::subrange<Iterator> {
  return std::ranges::subrange(t.first, t.second);
}

}  // namespace yk

#endif  // YK_UTIL_TO_SUBRANGE_STD_HPP
