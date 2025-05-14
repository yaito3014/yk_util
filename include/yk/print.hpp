#ifndef YK_PRINT_HPP
#define YK_PRINT_HPP

#include "yk/colorize.hpp"

#include <print>

#include <cstdio>

#if __cpp_lib_format >= 202311L

namespace yk {

template <class... Args>
inline void print(std::FILE* stream, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print(stream, "{}", colorize_and_format(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void print(std::ostream& os, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print(os, "{}", colorize_and_format(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void print(colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print("{}", colorize_and_format(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(std::FILE* stream, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println(stream, "{}", colorize_and_format(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(std::ostream& os, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println(os, "{}", colorize_and_format(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println("{}", colorize_and_format(fmt, std::forward<Args>(args)...));
}

}  // namespace yk

#endif

#endif  // YK_PRINT_HPP
