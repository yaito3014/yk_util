#ifndef YK_EXEC_WORKER_POOL_HPP
#define YK_EXEC_WORKER_POOL_HPP

#include "yk/interrupt_exception.hpp"

#include <algorithm>
#include <print>
#include <stop_token>
#include <thread>
#include <vector>
#include <stdexcept>
#include <exception>

namespace yk::exec {

using worker_id_t = unsigned;

class worker_pool {
public:
  worker_pool()
  {
    worker_limit_ = static_cast<int>(std::max(2u, std::thread::hardware_concurrency()));
  }

  ~worker_pool()
  {
    halt_and_clear();
  }

  void set_worker_limit(int worker_limit)
  {
    if (worker_limit < 2) {
      throw std::invalid_argument{"worker limit must be >= 2"};
    }
    worker_limit_ = worker_limit;
  }

  [[nodiscard]]
  int get_worker_limit() const noexcept { return worker_limit_; }

  [[nodiscard]]
  int launched_worker_count() const noexcept { return static_cast<int>(threads_.size()); }

  [[nodiscard]]
  std::stop_token stop_token() const noexcept { return stop_source_.get_token(); }

  [[nodiscard]]
  bool stop_requested() const noexcept { return stop_source_.stop_requested(); }

  void halt()
  {
    if (!stop_source_.stop_requested()) {
      stop_source_.request_stop();
    }
    stop_source_ = {};
  }

  void halt_and_clear()
  {
    halt();

    std::exception_ptr exception;

    for (auto& thread : threads_) {
      if (!thread.thread.joinable()) {
        continue;
      }

      thread.thread.join();

      if (thread.exception) {
        exception = thread.exception;
      }
    }

    threads_.clear();
    if (exception) {
      std::rethrow_exception(exception);
    }
  }

  template <class F>
  void launch(F&& f)
  {
    static_assert(std::invocable<F, worker_id_t, std::stop_token>);

    threads_.emplace_back(std::thread{[
      this,
      id = static_cast<worker_id_t>(threads_.size()),
      f = std::forward<F>(f)
    ]() mutable {
      try {
        f(id, stop_source_.get_token());

      } catch (const yk::interrupt_exception&) {
        // do nothing

      } catch (...) {
        stop_source_.request_stop();
        threads_[id].exception = std::current_exception();
      }
    }}, std::exception_ptr{});
  }

  template <class F>
  void launch_rest(F&& f)
  {
    const auto remaining = worker_limit_ - launched_worker_count();

    for (int i = 0; i < remaining; ++i) {
      launch(f);
    }
  }

private:
  int worker_limit_ = 2;

  struct ThreadData
  {
    std::thread thread;
    std::exception_ptr exception;
  };
  std::vector<ThreadData> threads_;
  std::stop_source stop_source_;
};

}  // namespace yk::exec

#endif
