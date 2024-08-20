#ifndef YK_UTIL_TO_SUBRANGE_TO_SUBRANGE_HPP
#define YK_UTIL_TO_SUBRANGE_TO_SUBRANGE_HPP

#include <ranges>
#include <tuple>

namespace yk {

template <class IterPair>
  requires(std::tuple_size_v<std::remove_cvref_t<IterPair>> == 2)
constexpr auto to_subrange(IterPair&& t) noexcept
    -> std::ranges::subrange<std::tuple_element_t<0, std::remove_cvref_t<IterPair>>, std::tuple_element_t<1, std::remove_cvref_t<IterPair>>> {
  return std::ranges::subrange(std::get<0>(std::forward<IterPair>(t)), std::get<1>(std::forward<IterPair>(t)));
}

}  // namespace yk

#endif  // YK_UTIL_TO_SUBRANGE_TO_SUBRANGE_HPP
