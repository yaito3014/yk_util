#ifndef YK_UTIL_HASH_BOOST_HPP
#define YK_UTIL_HASH_BOOST_HPP

#include <boost/container_hash/hash.hpp>

#include <cstddef>

namespace yk {

template <class T>
[[nodiscard]] constexpr std::size_t boost_hash_value_for(const T& x) noexcept {
  return boost::hash<T>{}(x);
}

}  // namespace yk

#endif  // YK_UTIL_HASH_BOOST_HPP
