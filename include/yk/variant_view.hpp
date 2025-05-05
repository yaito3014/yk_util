#ifndef YK_VARIANT_VIEW_HPP
#define YK_VARIANT_VIEW_HPP

#ifdef YK_VARIANT_INCLUDE_STL
#error "did you mean YK_VARIANT_INCLUDE_STD?"
#endif

#ifndef YK_VARIANT_INCLUDE_STD
#define YK_VARIANT_INCLUDE_STD 1
#endif

#if YK_VARIANT_INCLUDE_STD
#include "yk/variant_view/std.hpp"
#endif

#if YK_VARIANT_INCLUDE_BOOST
#include "yk/variant_view/boost.hpp"
#endif

#endif  // YK_VARIANT_VIEW_HPP
