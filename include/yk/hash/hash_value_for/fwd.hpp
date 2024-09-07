#ifndef YK_HASH_HASH_VALUE_FOR_FWD_HPP
#define YK_HASH_HASH_VALUE_FOR_FWD_HPP

#include <cstddef>

namespace yk {

template <class T>
[[nodiscard]] std::size_t hash_value_for(const T& x) noexcept;

}

#endif  // YK_HASH_HASH_VALUE_FOR_FWD_HPP