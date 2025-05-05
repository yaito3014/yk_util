#ifndef YK_VARIANT_VIEW_BOOST_HASH_HPP
#define YK_VARIANT_VIEW_BOOST_HASH_HPP

#include "yk/variant_view.hpp"

#include <boost/functional/hash.hpp>

#include <type_traits>

#include <cstddef>

namespace yk {

template <class Variant, class... Ts>
constexpr std::size_t hash_value(const yk::variant_view<Variant, Ts...>& view) {
  return hash_value(view.base());
}

}  // namespace yk

#endif  // YK_VARIANT_VIEW_BOOST_HASH_HPP
