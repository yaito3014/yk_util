#ifndef YK_ARCH_HPP
#define YK_ARCH_HPP

#include <new>

// ABI compatibility stabilizer

#if !defined(__clang__) && defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Winterference-size"
#endif

#ifndef YK_HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE
static_assert(64 == std::hardware_destructive_interference_size);
# define YK_HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE 64
#endif


#ifndef YK_HARDWARE_CONSTRUCTIVE_INTERFERENCE_SIZE
static_assert(64 == std::hardware_constructive_interference_size);
# define YK_HARDWARE_CONSTRUCTIVE_INTERFERENCE_SIZE 64
#endif

#if !defined(__clang__) && defined(__GNUC__)
# pragma GCC diagnostic pop
#endif


#if _MSC_VER
# define YK_FORCEALIGN_BEGIN \
  _Pragma("warning(push)") \
  _Pragma("warning(disable: 4324)")
#else
# define YK_FORCEALIGN_BEGIN
#endif

#if _MSC_VER
# define YK_FORCEALIGN_END \
  _Pragma("warning(pop)")
#else
# define YK_FORCEALIGN_END
#endif


namespace yk {

inline constexpr std::size_t hardware_destructive_interference_size = YK_HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE;
inline constexpr std::size_t hardware_constructive_interference_size = YK_HARDWARE_CONSTRUCTIVE_INTERFERENCE_SIZE;

} // yk

#endif
