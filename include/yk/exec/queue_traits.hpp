#ifndef YK_EXEC_QUEUE_TRAITS
#define YK_EXEC_QUEUE_TRAITS

#include "yk/exec/debug.hpp"// for ODR violation safety

#include <version>

#if __cpp_lib_jthread >= 201911L
#include <stop_token>
#endif

namespace yk::exec {

template <class QueueT>
struct queue_traits
{
  using queue_type = QueueT;
  using value_type = typename QueueT::value_type;

  // You need to provide this
  // static constexpr bool need_stop_token_for_cancel = true;

#if __cpp_lib_jthread >= 201911L
  // [if need_stop_token_for_cancel is true]
  // You need to provide this
  template <class... Args>
  [[nodiscard]] static bool cancelable_bounded_push(std::stop_token const& stop_token, queue_type& queue, Args&&... args) = delete;

  // [if need_stop_token_for_cancel is true]
  // You need to provide this
  [[nodiscard]] static bool cancelable_pop(std::stop_token const& stop_token, queue_type& queue, value_type& value) = delete;
#endif

  // [if need_stop_token_for_cancel is false]
  // You need to provide this
  template <class... Args>
  [[nodiscard]] static bool cancelable_bounded_push(queue_type& queue, Args&&... args) = delete;

  // [if need_stop_token_for_cancel is false]
  // You need to provide this
  [[nodiscard]] static bool cancelable_pop(queue_type& queue, value_type& value) = delete;
};

} // yk::exec

#endif
