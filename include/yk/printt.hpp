#ifndef YK_PRINTT_HPP
#define YK_PRINTT_HPP

#include "yk/detail/traced_type.hpp"

#include <boost/exception/get_error_info.hpp>
#include <boost/stacktrace.hpp>

#include <exception>
#include <iostream>
#include <string>

namespace yk {

void printt(std::ostream& os, const std::exception& e)
{
  if (const boost::stacktrace::stacktrace* const stacktrace = boost::get_error_info<detail::traced_type>(e)) {
    os << e.what() << std::endl;
    os << boost::stacktrace::to_string(*stacktrace) << std::endl;
  } else {
    os << e.what() << std::endl;
  }

  try {
    std::rethrow_if_nested(e);
  } catch (const std::exception& e) {
    printt(os, e);
  } catch (...) {
    throw;
  }
}

void printt(const std::exception& e)
{
  printt(std::cout, e);
}

}  // namespace yk

#endif  // YK_PRINTT_HPP
