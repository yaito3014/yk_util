#ifndef YK_UTIL_PACK_INDEXING_HPP
#define YK_UTIL_PACK_INDEXING_HPP

#include <cstddef>

namespace yk {

template <std::size_t I, class... Ts>
struct pack_indexing {};

template <std::size_t I, class T, class... Ts>
struct pack_indexing<I, T, Ts...> : pack_indexing<I - 1, Ts...> {};

template <class T, class... Ts>
struct pack_indexing<0, T, Ts...> {
  using type = T;
};

template <std::size_t I, class... Ts>
using pack_indexing_t = typename pack_indexing<I, Ts...>::type;

}  // namespace yk

#endif  // YK_UTIL_PACK_INDEXING_HPP
