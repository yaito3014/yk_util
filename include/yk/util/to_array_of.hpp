#ifndef YK_UTIL_TO_ARRAY_OF_HPP
#define YK_UTIL_TO_ARRAY_OF_HPP

#include <array>
#include <concepts>
#include <utility>

namespace yk {

template <class T, std::convertible_to<T>... Us>
constexpr std::array<T, sizeof...(Us)> to_array_of(Us&&... xs) {
  return std::array<T, sizeof...(Us)>{static_cast<T>(std::forward<Us>(xs))...};
}

}  // namespace yk

#endif  // YK_UTIL_TO_ARRAY_OF_HPP
