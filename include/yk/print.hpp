#ifndef YK_PRINT_HPP
#define YK_PRINT_HPP

#include "yk/colorize.hpp"

#include <ostream>
#include <print>

#include <cstdio>

namespace yk {

template <class... Args>
inline void print(std::FILE* stream, const colorize_config& cfg, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print(stream, "{}", format_colorize(cfg, fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void print(std::FILE* stream, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print(stream, "{}", format_colorize(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void print(std::ostream& os, const colorize_config& cfg, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print(os, "{}", format_colorize(cfg, fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void print(std::ostream& os, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print(os, "{}", format_colorize(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void print(const colorize_config& cfg, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print("{}", format_colorize(cfg, fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void print(colorize_format_string<Args...> fmt, Args&&... args)
{
  std::print("{}", format_colorize(stdout_config, fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(std::FILE* stream, const colorize_config& cfg, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println(stream, "{}", format_colorize(cfg, fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(std::FILE* stream, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println(stream, "{}", format_colorize(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(std::ostream& os, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println(os, "{}", format_colorize(fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(const colorize_config& cfg, colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println("{}", format_colorize(cfg, fmt, std::forward<Args>(args)...));
}

template <class... Args>
inline void println(colorize_format_string<Args...> fmt, Args&&... args)
{
  std::println("{}", format_colorize(stdout_config, fmt, std::forward<Args>(args)...));
}

inline void println() { std::println(); }

}  // namespace yk

#endif  // YK_PRINT_HPP
