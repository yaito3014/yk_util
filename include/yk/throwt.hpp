#ifndef YK_THROWT_HPP
#define YK_THROWT_HPP

#include "yk/detail/string_like.hpp"
#include "yk/detail/traced_type.hpp"

#include <boost/stacktrace/stacktrace.hpp>

#include <boost/exception/info.hpp>

#include <boost/exception/exception.hpp>

#include <exception>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#ifndef YK_THROWT_THROW

#define YK_THROWT_THROW(...) throw __VA_ARGS__

#endif  // YK_THROWT_THROW

#ifndef YK_THROWT_NORETURN

#define YK_THROWT_NORETURN [[noreturn]]

#endif  // YK_THROWT_NORETURN

namespace yk {

namespace detail {

template <class T, class... Args>
concept constructible_from_string_like_types =
    (
         std::is_constructible_v<T, Args..., std::string>
      || std::is_constructible_v<T, Args..., std::string_view>
      || std::is_constructible_v<T, Args..., const char*>
    );
    
template <class T>
concept NotStringLike = !StringLike<T>;

}  // namespace detail

inline namespace error_functions {

template <class E>
YK_THROWT_NORETURN void throwt() {
  static_assert(std::is_base_of_v<std::exception, E>);
  static_assert(std::is_constructible_v<E>);
  YK_THROWT_THROW(boost::enable_error_info(E{}) << detail::traced_type{boost::stacktrace::stacktrace()});
}

template <class E, class Arg, class... Rest>
  requires std::is_constructible_v<E, Arg, Rest...>
YK_THROWT_NORETURN void throwt(Arg&& arg, Rest&&... rest) {
  static_assert(std::is_base_of_v<std::exception, E>);
  static_assert(!std::is_base_of_v<std::exception, std::remove_cvref_t<Arg>>, "don't copy/move construct exception types directly");
  YK_THROWT_THROW(
      boost::enable_error_info(E{std::forward<Arg>(arg), std::forward<Rest>(rest)...})
      << detail::traced_type{boost::stacktrace::stacktrace()}
  );
}

template <class E, class... Args>
  requires detail::constructible_from_string_like_types<E>
YK_THROWT_NORETURN void throwt(std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::is_base_of_v<std::exception, E>);
  YK_THROWT_THROW(
      boost::enable_error_info(E{std::format(std::move(fmt), std::forward<Args>(args)...)})
      << detail::traced_type{boost::stacktrace::stacktrace()}
  );
}

template <class E, detail::NotStringLike Arg0, class... Args>
  requires detail::constructible_from_string_like_types<E, Arg0>
YK_THROWT_NORETURN void throwt(Arg0&& arg0, std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::is_base_of_v<std::exception, E>);
  YK_THROWT_THROW(
      boost::enable_error_info(E{std::forward<Arg0>(arg0), std::format(std::move(fmt), std::forward<Args>(args)...)})
      << detail::traced_type{boost::stacktrace::stacktrace()}
  );
}

template <class E, detail::NotStringLike Arg0, detail::NotStringLike Arg1, class... Args>
  requires detail::constructible_from_string_like_types<E, Arg0, Arg1>
YK_THROWT_NORETURN void throwt(Arg0&& arg0, Arg1&& arg1, std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::is_base_of_v<std::exception, E>);
  YK_THROWT_THROW(
      boost::enable_error_info(
          E{std::forward<Arg0>(arg0), std::forward<Arg1>(arg1),
            std::format(std::move(fmt), std::forward<Args>(args)...)}
      )
      << detail::traced_type{boost::stacktrace::stacktrace()}
  );
}

}  // namespace error_functions

}  // namespace yk

#endif  // YK_THROWT_HPP
