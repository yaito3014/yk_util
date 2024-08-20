#ifndef YK_UTIL_REVERSE_HPP
#define YK_UTIL_REVERSE_HPP

#include <compare>

namespace yk {

// note: Don't use this in projection with `std::forward_as_tuple` as it creates dangling reference
template <class T>
struct [[nodiscard]] reverse {
  T val;

  [[nodiscard]] constexpr auto operator<=>(const reverse& other) const noexcept(noexcept(other.val <=> val)) -> std::compare_three_way_result_t<T> {
    return other.val <=> val;
  }

  [[nodiscard]] constexpr bool operator==(const reverse& other) const noexcept(noexcept(val == other.val)) { return val == other.val; }
};

template <class T>
reverse(T&) -> reverse<const T&>;

template <class T>
reverse(T&&) -> reverse<T>;

}  // namespace yk

#endif  // YK_UTIL_REVERSE_HPP
