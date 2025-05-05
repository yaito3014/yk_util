#ifndef YK_EXEC_PORT_DETAIL_BOOST_LOCKFREE_COMMON_HPP
#define YK_EXEC_PORT_DETAIL_BOOST_LOCKFREE_COMMON_HPP

#include "yk/exec/queue_traits.hpp"

#include <stop_token>
#include <utility>

namespace yk::exec {

namespace detail {

template <class BoostLockfreeT>
struct boost_lockfree_common_traits;

template <template <class T, class... Options> class BoostLockfreeT, class T, class... Options>
struct boost_lockfree_common_traits<BoostLockfreeT<T, Options...>>
{
  using queue_type = BoostLockfreeT<T, Options...>;
  using value_type = T;

  static constexpr bool need_stop_token_for_cancel = true;

  template <class... Args>
  [[nodiscard]]
  static bool cancelable_bounded_push(std::stop_token const& stop_token, queue_type& queue, Args&&... args)
  {
    while (!stop_token.stop_requested()) {
      if (queue.bounded_push(std::forward<Args>(args)...)) return true;
    }
    return false;
  }

  template <class... Args>
  [[nodiscard]]
  static bool cancelable_push(std::stop_token const& stop_token, queue_type& queue, Args&&... args)
  {
    while (!stop_token.stop_requested()) {
      if (queue.push(std::forward<Args>(args)...)) return true;
    }
    return false;
  }

  [[nodiscard]]
  static bool cancelable_pop(std::stop_token const& stop_token, queue_type& queue, value_type& value)
  {
    while (!stop_token.stop_requested()) {
      if (queue.pop(value)) return true;
    }
    return false;
  }
};

} // detail

} // yk::exec

#endif
