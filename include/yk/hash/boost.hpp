#ifndef YK_HASH_BOOST_HPP
#define YK_HASH_BOOST_HPP

#include "yk/hash/hash.hpp"

#include <boost/container_hash/hash_fwd.hpp>

#include <cstddef>

namespace yk {

template <class T>
[[nodiscard]] constexpr std::size_t boost_hash_value_for(const T& x) noexcept {
  return boost::hash<T>{}(x);
}

}  // namespace yk

#endif  // YK_HASH_BOOST_HPP
