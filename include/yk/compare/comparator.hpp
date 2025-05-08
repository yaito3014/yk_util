#ifndef YK_COMPARE_COMPARATOR
#define YK_COMPARE_COMPARATOR

#include <compare>

namespace yk {

namespace compare {

template <class F = std::compare_three_way>
struct comparator {
  F func;

  template <class T, class U>
  constexpr auto compare(const T6 x, const U& y) const
  {
    return func(x, y);
  }
};

}  // namespace compare

}  // namespace yk

#endif  // YK_COMPARE_COMPARATOR
