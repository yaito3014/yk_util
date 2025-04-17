#ifndef YK_MAYBE_MUTEX_HPP
#define YK_MAYBE_MUTEX_HPP

#include <version>

#if __cpp_lib_parallel_algorithm >= 201603L

#include <execution>
#include <mutex>

namespace yk {

template <class Policy>
struct is_parallel {
  static_assert(std::is_execution_policy_v<Policy>, "Policy must be execution policy");
  // static_assert(yk::always_false_v<Policy>, "unhandled execution policy");
};

template <>
struct is_parallel<std::execution::sequenced_policy> : std::false_type {};

template <>
struct is_parallel<std::execution::parallel_policy> : std::true_type {};

template <class Policy>
constexpr bool is_parallel_v = is_parallel<Policy>::value;

namespace xo {  // exposition only

// https://en.cppreference.com/w/cpp/named_req/BasicLockable
template <class M>
concept BasicLockable = requires(M& m) {
  m.lock();
  m.unlock();
};

// https://en.cppreference.com/w/cpp/named_req/Lockable
template <class M>
concept Lockable = BasicLockable<M> && requires(M& m) {
  { m.try_lock() } -> std::convertible_to<bool>;
};

}  // namespace xo

template <class Mutex, class Policy>
class maybe_mutex;

template <class Mutex, class Policy>
  requires is_parallel_v<Policy>
class maybe_mutex<Mutex, Policy> : public Mutex {
public:
  static_assert(xo::Lockable<Mutex>);
  using Mutex::Mutex;
};

template <class Mutex, class Policy>
  requires (!is_parallel_v<Policy>)
class maybe_mutex<Mutex, Policy> {
public:
  static_assert(xo::Lockable<Mutex>);

  constexpr void lock() const noexcept {}
  constexpr void unlock() const noexcept {}
  [[nodiscard]] constexpr bool try_lock() const noexcept { return true; }
};

}  // namespace yk

#endif  // __cpp_lib_parallel_algorithm >= 201603L

#endif  // YK_MAYBE_MUTEX_HPP
