#ifndef YK_ARRAY_HPP
#define YK_ARRAY_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <type_traits>

namespace yk {

template <class T, std::size_t... Sizes>
  requires std::copy_constructible<T>
[[nodiscard]] constexpr std::array<T, (Sizes + ...)> concat(const std::array<T, Sizes>&... arrs)  //
    noexcept(std::is_nothrow_copy_constructible_v<T>) {
  std::array<T, (Sizes + ...)> res;
  std::size_t i = 0;

  ((std::copy_n(arrs.begin(), Sizes, res.begin() + i), i += Sizes), ...);

  return res;
}

}  // namespace yk

#endif  // YK_ARRAY_HPP
