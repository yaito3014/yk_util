#ifndef YK_UTIL_TYPE_LIST_HPP
#define YK_UTIL_TYPE_LIST_HPP

#include <cstdint>

namespace yk {

template <class... Ts>
struct type_list {
  static constexpr std::size_t size = sizeof...(Ts);
};

}  // namespace yk

#endif  // YK_UTIL_TYPE_LIST_HPP
