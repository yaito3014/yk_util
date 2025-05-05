#ifndef YK_VARIANT_VIEW_HASH_HPP
#define YK_VARIANT_VIEW_HASH_HPP

#include "yk/variant_view.hpp"

#if YK_VARIANT_INCLUDE_STD
#include "yk/variant_view/hash/std_hash.hpp"
#endif

#if YK_VARIANT_INCLUDE_BOOST
#include "yk/variant_view/hash/boost_hash.hpp"
#endif

#endif  // YK_VARIANT_VIEW_HASH_HPP
