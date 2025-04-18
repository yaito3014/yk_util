#ifndef YK_PAR_FOR_EACH_HPP
#define YK_PAR_FOR_EACH_HPP

#include <version>

#if __cpp_lib_parallel_algorithm >= 201603L

#include <algorithm>
#include <exception>
#include <execution>
#include <functional>
#include <ranges>
#include <shared_mutex>
#include <type_traits>
#include <utility>

namespace yk {

template <class JobPolicy>
struct is_job_policy : std::false_type {};

template <class JobPolicy>
inline constexpr bool is_job_policy_v = is_job_policy<JobPolicy>::value;

namespace execution {

struct abort_policy {};
struct proceed_policy {};

inline constexpr abort_policy abort{};
inline constexpr proceed_policy proceed{};

}  // namespace execution

template <>
struct is_job_policy<execution::abort_policy> : std::true_type {};

template <>
struct is_job_policy<execution::proceed_policy> : std::true_type {};

template <class JobPolicy, class Policy, std::forward_iterator ForwardIterator, class Func>
void for_each(JobPolicy&&, Policy&& policy, ForwardIterator first, ForwardIterator last, Func func) {
  static_assert(is_job_policy_v<std::remove_cvref_t<JobPolicy>>);
  static_assert(std::is_execution_policy_v<std::remove_cvref_t<Policy>>);
  static_assert(std::is_copy_constructible_v<Func>, "std::for_each requires func to be CopyConstructible");
  std::shared_mutex mtx;
  std::exception_ptr exception;
  const auto wrapper = [&, func = std::move(func)]<class T>(T&& arg) noexcept {
    try {
      if constexpr (std::is_same_v<std::remove_cvref_t<JobPolicy>, execution::abort_policy>) {
        std::shared_lock lock{mtx};
        if (exception) return;
      }
      std::invoke(func, std::forward<T>(arg));
    } catch (...) {
      std::lock_guard lock{mtx};
      exception = std::current_exception();
    }
  };
  std::for_each(std::forward<Policy>(policy), first, last, wrapper);
  if (exception) std::rethrow_exception(exception);
}

template <class Policy, std::forward_iterator ForwardIterator, class Func>
void for_each(Policy&& policy, ForwardIterator first, ForwardIterator last, Func func) {
  for_each(execution::abort, std::forward<Policy>(policy), first, last, std::move(func));
}

namespace ranges {

template <class JobPolicy, class Policy, std::ranges::forward_range R, class Func>
void for_each(JobPolicy&& job_policy, Policy&& policy, R&& r, Func func) {
  ::yk::for_each(std::forward<JobPolicy>(job_policy), std::forward<Policy>(policy), std::ranges::begin(std::forward<R>(r)),
                 std::ranges::end(std::forward<R>(r)), std::move(func));
}

template <class Policy, std::ranges::forward_range R, class Func>
void for_each(Policy&& policy, R&& r, Func func) {
  for_each(execution::abort, std::forward<Policy>(policy), std::forward<R>(r), std::move(func));
}

template <class JobPolicy, class Policy, std::forward_iterator I, std::sentinel_for<I> S, class Func>
void for_each(JobPolicy&& job_policy, Policy&& policy, I iter, S sent, Func func) {
  std::ranges::common_view common_view(iter, sent);
  ::yk::for_each(std::forward<JobPolicy>(job_policy), std::forward<Policy>(policy), common_view.begin(), common_view.end(), std::move(func));
}

template <class Policy, std::forward_iterator I, std::sentinel_for<I> S, class Func>
void for_each(Policy&& policy, I iter, S sent, Func func) {
  for_each(execution::abort, std::forward<Policy>(policy), iter, sent, std::move(func));
}

}  // namespace ranges

}  // namespace yk

#endif  // __cpp_lib_parallel_algorithm

#endif  // YK_PAR_FOR_EACH_HPP
