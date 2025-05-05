#ifndef YK_LIFETIMEBOUND_HPP
#define YK_LIFETIMEBOUND_HPP

#ifndef YK_LIFETIMEBOUND

#if defined(__clang__)

#define YK_LIFETIMEBOUND [[clang::lifetimebound]]

#elif defined(_MSC_VER)

#define YK_LIFETIMEBOUND [[msvc::lifetimebound]]

#else

#define YK_LIFETIMEBOUND

#endif

#endif

#endif  // YK_LIFETIMEBOUND_HPP
