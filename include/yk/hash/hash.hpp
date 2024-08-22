#ifndef YK_UTIL_HASH_HASH_HPP
#define YK_UTIL_HASH_HASH_HPP

#define YK_PP_REQUIRE_SEMICOLON static_assert(true)

#define YK_ADAPT_HASH(ns, type, param, hash_stmt)                                               \
  namespace std {                                                                               \
  template <>                                                                                   \
  struct hash<::ns::type> {                                                                     \
    constexpr size_t operator()(::ns::type const& param) const noexcept /* strengthened */      \
    { hash_stmt };                                                                              \
  };                                                                                            \
  } /* std */                                                                                   \
                                                                                                \
  namespace ns {                                                                                \
  [[nodiscard]] constexpr std::size_t hash_value(type const& value) noexcept /* strengthened */ \
  {                                                                                             \
    return ::yk::hash_value_for(value);                                                         \
  }                                                                                             \
  } /* ns */                                                                                    \
  YK_PP_REQUIRE_SEMICOLON

#define YK_PP_EXPAND(...) __VA_ARGS__
#define YK_PP_REMOVE_PAREN(expr) YK_PP_EXPAND expr

#define YK_ADAPT_HASH_TEMPLATE(ns, parenthesized_type, param, hash_stmt, ...)                                                     \
  namespace std {                                                                                                                 \
  template <__VA_ARGS__>                                                                                                          \
  struct hash<::ns::YK_PP_REMOVE_PAREN(parenthesized_type)> {                                                                     \
    constexpr size_t operator()(::ns::YK_PP_REMOVE_PAREN(parenthesized_type) const& param) const noexcept /* strengthened */      \
    { hash_stmt };                                                                                                                \
  };                                                                                                                              \
  } /* std */                                                                                                                     \
                                                                                                                                  \
  namespace ns {                                                                                                                  \
  template <__VA_ARGS__>                                                                                                          \
  [[nodiscard]] constexpr std::size_t hash_value(YK_PP_REMOVE_PAREN(parenthesized_type) const& value) noexcept /* strengthened */ \
  {                                                                                                                               \
    return ::yk::hash_value_for(value);                                                                                           \
  }                                                                                                                               \
  } /* ns */                                                                                                                      \
  YK_PP_REQUIRE_SEMICOLON

#endif  // YK_UTIL_HASH_HASH_HPP
