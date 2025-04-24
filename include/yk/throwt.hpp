#include "yk/detail/string_like.hpp"

#include <boost/exception/all.hpp>
#include <boost/stacktrace/stacktrace.hpp>

#include <exception>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace yk {

namespace detail {

using traced_type = boost::error_info<struct stacktrace_tag, boost::stacktrace::stacktrace>;

}  // namespace detail

inline namespace error_functions {

template <class E>
[[noreturn]] void throwt() {
  static_assert(std::derived_from<E, std::exception>);
  static_assert(std::constructible_from<E>);
  throw boost::enable_error_info(E{}) << detail::traced_type{boost::stacktrace::stacktrace()};
}

template <class E, class Arg, class... Rest>
  requires std::constructible_from<E, Arg, Rest...> && (!detail::StringLike<Arg>)
[[noreturn]] void throwt(Arg&& arg, Rest&&... rest) {
  static_assert(std::derived_from<E, std::exception>);
  throw boost::enable_error_info(E{std::forward<Arg>(arg), std::forward<Rest>(rest)...})
      << detail::traced_type{boost::stacktrace::stacktrace()};
}

template <class E, class... Args>
[[noreturn]] void throwt(std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::derived_from<E, std::exception>);
  static_assert(std::is_constructible_v<E, std::string>);
  throw boost::enable_error_info(E{std::format(std::move(fmt), std::forward<Args>(args)...)})
      << detail::traced_type{boost::stacktrace::stacktrace()};
}

template <class E, class Arg0, class... Args>
[[noreturn]] void throwt(Arg0&& arg0, std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::derived_from<E, std::exception>);
  static_assert(std::is_constructible_v<E, Arg0, std::string>);
  throw boost::enable_error_info(E{std::forward<Arg0>(arg0), std::format(std::move(fmt), std::forward<Args>(args)...)})
      << detail::traced_type{boost::stacktrace::stacktrace()};
}

template <class E, class Arg0, class Arg1, class... Args>
[[noreturn]] void throwt(Arg0&& arg0, Arg1&& arg1, std::format_string<Args...> fmt, Args&&... args) {
  static_assert(std::derived_from<E, std::exception>);
  static_assert(std::is_constructible_v<E, Arg0, Arg1, std::string>);
  throw boost::enable_error_info(
      E{std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::format(std::move(fmt), std::forward<Args>(args)...)}
  ) << detail::traced_type{boost::stacktrace::stacktrace()};
}

}  // namespace error_functions

}  // namespace yk
