#ifndef YK_UTIL_TO_UNDERLYING_HPP
#define YK_UTIL_TO_UNDERLYING_HPP

#include <utility>

namespace yk {

template <class T>
[[nodiscard]] constexpr std::underlying_type_t<T> to_underlying(T value) noexcept {
  return static_cast<std::underlying_type_t<T>>(value);
}

}  // namespace yk

#endif  // YK_UTIL_TO_UNDERLYING_HPP
