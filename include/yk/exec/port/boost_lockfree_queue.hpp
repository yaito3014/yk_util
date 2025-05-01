#ifndef YK_EXEC_PORT_BOOST_LOCKFREE_QUEUE_HPP
#define YK_EXEC_PORT_BOOST_LOCKFREE_QUEUE_HPP

#include "yk/exec/port/detail/boost_lockfree_common.hpp"

#include <boost/lockfree/queue.hpp>

namespace yk::exec {

template <class T, class... Options>
struct queue_access<boost::lockfree::queue<T, Options...>>
  : detail::boost_lockfree_common_access<boost::lockfree::queue<T, Options...>>
{};

template <class T, class... Options>
struct queue_traits<boost::lockfree::queue<T, Options...>>
  : detail::boost_lockfree_common_traits<boost::lockfree::queue<T, Options...>>
{};

} // yk::exec

#endif
