#ifndef YK_DETAIL_STRING_LIKE_HPP
#define YK_DETAIL_STRING_LIKE_HPP

#include <string_view>

namespace yk::detail {

// (note: we need a solid definition for "string like", whatever that means)
template <class T>
concept StringLike = requires(T x) { std::basic_string_view{x}; };

}  // namespace yk::detail

#endif  // YK_DETAIL_STRING_LIKE_HPP
