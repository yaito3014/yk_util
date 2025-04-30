#ifndef YK_EXEC_THREAD_INDEX_HPP
#define YK_EXEC_THREAD_INDEX_HPP

#include <cstddef>


namespace yk::exec {

// 0-based thread index, guaranteed to fit inside [0, worker_count)
// This can be safely used for indexing contiguous array of thread-local data:
// e.g. my_worker_datas[id]
using thread_index_t = std::size_t;

} // yk::exec

#endif
