#ifndef YK_EXEC_QUEUE_TRAITS
#define YK_EXEC_QUEUE_TRAITS

#include "yk/exec/debug.hpp"// for ODR violation safety

namespace yk::exec {

template <class QueueT>
struct queue_traits
{
  using queue_type = QueueT;
  using value_type = typename QueueT::value_type;

  // TODO: auto detection
};


template <class QueueT>
struct queue_access
{
  using queue_type = QueueT;
  using traits_type = queue_traits<QueueT>;
  using value_type = typename traits_type::value_type;

  // TODO: auto detection
};

} // yk::exec

#endif
