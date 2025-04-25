#ifndef YK_DETAIL_TRACED_TYPE
#define YK_DETAIL_TRACED_TYPE

#include <boost/exception/error_info.hpp>
#include <boost/stacktrace/stacktrace_fwd.hpp>

namespace yk {

namespace detail {

using traced_type = boost::error_info<struct stacktrace_tag, boost::stacktrace::stacktrace>;

}  // namespace detail

}  // namespace yk

#endif  // YK_DETAIL_TRACED_TYPE
