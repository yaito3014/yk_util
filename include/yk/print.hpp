#ifndef YK_PRINT_HPP
#define YK_PRINT_HPP

#include "yk/colorize.hpp"

#include <print>

#if __cpp_lib_format >= 202311L

namespace yk {

template <class... Args>
inline void print(std::FILE* stream, colored_format_string<Args...> fmt, Args&&... args)
{
  std::print(stream, std::runtime_format(fmt.get()), std::forward<Args>(args)...);
}

template <class... Args>
inline void print(std::ostream& os, colored_format_string<Args...> fmt, Args&&... args)
{
  std::print(os, std::runtime_format(fmt.get()), std::forward<Args>(args)...);
}

template <class... Args>
inline void print(colored_format_string<Args...> fmt, Args&&... args)
{
  std::print(std::runtime_format(fmt.get()), std::forward<Args>(args)...);
}

template <class... Args>
inline void println(std::FILE* stream, colored_format_string<Args...> fmt, Args&&... args)
{
  std::println(stream, std::runtime_format(fmt.get()), std::forward<Args>(args)...);
}

template <class... Args>
inline void println(std::ostream& os, colored_format_string<Args...> fmt, Args&&... args)
{
  std::println(os, std::runtime_format(fmt.get()), std::forward<Args>(args)...);
}

template <class... Args>
inline void println(colored_format_string<Args...> fmt, Args&&... args)
{
  std::println(std::runtime_format(fmt.get()), std::forward<Args>(args)...);
}

}  // namespace yk

#endif

#endif  // YK_PRINT_HPP
