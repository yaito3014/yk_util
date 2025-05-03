#ifndef YK_NO_UNIQUE_ADDRESS_HPP
#define YK_NO_UNIQUE_ADDRESS_HPP

// <https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch/#c++20-[[no_unique_address]]>

#if _MSC_VER && _MSC_VER < 1929 // VS 2019 v16.9 or before
# error "Too old MSVC version; we don't support this because it leads to ODR violation regarding the existence of [[(msvc::)no_unique_address]]"
#endif

#if _MSC_VER && __INTELLISENSE__ // Memory Layout view shows wrong layout without this workaround
# define YK_NO_UNIQUE_ADDRESS [[msvc::no_unique_address, no_unique_address]]

#elif _MSC_VER // normal MSVC
# define YK_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]

#else // other compilers
# define YK_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#endif
