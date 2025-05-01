#ifndef YK_EXEC_PORT_BOOST_LOCKFREE_STACK_HPP
#define YK_EXEC_PORT_BOOST_LOCKFREE_STACK_HPP

#include "yk/exec/port/detail/boost_lockfree_common.hpp"

#include <boost/lockfree/stack.hpp>

namespace yk::exec {

template <class T, class... Options>
struct queue_access<boost::lockfree::stack<T, Options...>>
  : detail::boost_lockfree_common_access<boost::lockfree::stack<T, Options...>>
{};

template <class T, class... Options>
struct queue_traits<boost::lockfree::stack<T, Options...>>
  : detail::boost_lockfree_common_traits<boost::lockfree::stack<T, Options...>>
{};

} // yk::exec

#endif
