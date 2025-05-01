#ifndef YK_EXEC_CV_QUEUE_TYPES_HPP
#define YK_EXEC_CV_QUEUE_TYPES_HPP

#include "yk/exec/debug.hpp" // for ODR violation safety

#include <type_traits>

#include <cstddef>

namespace yk::exec {

namespace detail {
using cv_queue_size_type = std::make_signed_t<std::size_t>;
} // detail

} // yk::exec

#endif
