#ifndef YK_EXEC_PORT_BOOST_LOCKFREE_QUEUE_HPP
#define YK_EXEC_PORT_BOOST_LOCKFREE_QUEUE_HPP

#include "yk/exec/queue_traits.hpp"

#include <boost/lockfree/queue.hpp>

#include <stop_token>

namespace yk::exec {

template <class T, class... Options>
struct queue_traits<boost::lockfree::queue<T, Options...>>
{
  static constexpr bool need_stop_token_for_cancel = true;
};

template <class T, class... Options>
struct queue_access<boost::lockfree::queue<T, Options...>>
{
  using queue_type = boost::lockfree::queue<T, Options...>;
  using traits_type = queue_traits<queue_type>;

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
  static bool cancelable_pop(std::stop_token const& stop_token, queue_type& queue, T& value)
  {
    while (!stop_token.stop_requested()) {
      if (queue.pop(value)) return true;
    }
    return false;
  }
};

} // yk::exec

#endif
