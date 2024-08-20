#ifndef YK_UTIL_TO_SUBRANGE_TO_SUBRANGE_HPP
#define YK_UTIL_TO_SUBRANGE_TO_SUBRANGE_HPP

#include <ranges>
#include <utility>

namespace yk {

template <class Iterator>
constexpr auto to_subrange(const std::pair<Iterator, Iterator>& t) noexcept -> std::ranges::subrange<Iterator> {
  return std::ranges::subrange(t.first, t.second);
}

}  // namespace yk

#endif  // YK_UTIL_TO_SUBRANGE_TO_SUBRANGE_HPP
