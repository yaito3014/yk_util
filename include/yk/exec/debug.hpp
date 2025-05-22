#ifndef YK_EXEC_DEBUG_HPP
#define YK_EXEC_DEBUG_HPP

#if !defined(YK_EXEC_DEBUG)
#if defined(NDEBUG)
#define YK_EXEC_DEBUG 0
#else
#define YK_EXEC_DEBUG 1
#endif
#endif

#if !defined(YK_EXEC_DEBUG_PRINT)
#if defined(YK_EXEC_DEBUG)
#define YK_EXEC_DEBUG_PRINT(...) __VA_ARGS__
#else
#define YK_EXEC_DEBUG_PRINT(...) do {} while (false)
#endif
#endif

namespace yk::exec {



}  // namespace yk::exec

#endif
