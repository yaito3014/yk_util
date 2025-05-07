#ifndef YK_PRINTT_HPP
#define YK_PRINTT_HPP

#include <version>

#if __cpp_lib_print >= 202207L

#include "yk/detail/traced_type.hpp"

#include <boost/exception/get_error_info.hpp>
#include <boost/stacktrace.hpp>

#include <exception>
#include <iostream>
#include <print>
#include <string>

namespace yk {

inline void printt(std::ostream& os, const std::exception& e)
{
  std::println(os, "{}", e.what());

  if (const boost::stacktrace::stacktrace* const stacktrace = boost::get_error_info<detail::traced_type>(e)) {
    std::println(os, "{}", boost::stacktrace::to_string(*stacktrace));
  }

  try {
    std::rethrow_if_nested(e);
  } catch (const std::exception& e) {
    printt(os, e);
  } catch (...) {
    throw;
  }
}

inline void printt(const std::exception& e)
{
  printt(std::cout, e);
}

}  // namespace yk

#endif

#endif  // YK_PRINTT_HPP
