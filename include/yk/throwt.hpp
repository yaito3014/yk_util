#ifndef YK_THROWT_HPP
#define YK_THROWT_HPP

#include "yk/detail/string_like.hpp"

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

#endif

namespace yk {

namespace detail {

using traced_type = boost::error_info<struct stacktrace_tag, boost::stacktrace::stacktrace>;

}  // namespace detail

inline namespace error_functions {

template <class E>
[[noreturn]] void throwt() {
  static_assert(std::is_base_of_v<std::exception, E>);
  static_assert(std::is_constructible_v<E>);
  YK_THROWT_THROW(boost::enable_error_info(E{}) << detail::traced_type{boost::stacktrace::stacktrace()});
}

template <class E, class Arg, class... Rest>
  requires std::is_constructible_v<E, Arg, Rest...> && (!detail::StringLike<Arg>)
[[noreturn]] void throwt(Arg&& arg, Rest&&... rest) {
  static_assert(std::is_base_of_v<std::exception, E>);
  YK_THROWT_THROW(
      boost::enable_error_info(E{std::forward<Arg>(arg), std::forward<Rest>(rest)...})
      << detail::traced_type{boost::stacktrace::stacktrace()}
  );
}

template <class E, class... Args>
[[noreturn]] void throwt(std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::is_base_of_v<std::exception, E>);
  static_assert(std::is_constructible_v<E, std::string>);
  YK_THROWT_THROW(
      boost::enable_error_info(E{std::format(std::move(fmt), std::forward<Args>(args)...)})
      << detail::traced_type{boost::stacktrace::stacktrace()}
  );
}

template <class E, class Arg0, class... Args>
[[noreturn]] void throwt(Arg0&& arg0, std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::is_base_of_v<std::exception, E>);
  static_assert(std::is_constructible_v<E, Arg0, std::string>);
  YK_THROWT_THROW(
      boost::enable_error_info(E{std::forward<Arg0>(arg0), std::format(std::move(fmt), std::forward<Args>(args)...)})
      << detail::traced_type{boost::stacktrace::stacktrace()}
  );
}

template <class E, class Arg0, class Arg1, class... Args>
[[noreturn]] void throwt(Arg0&& arg0, Arg1&& arg1, std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::is_base_of_v<std::exception, E>);
  static_assert(std::is_constructible_v<E, Arg0, Arg1, std::string>);
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

#endif

