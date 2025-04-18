#ifndef YK_EXEC_WORKER_POOL_HPP
#define YK_EXEC_WORKER_POOL_HPP

#include "yk/interrupt_exception.hpp"

#include <algorithm>
#include <print>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <vector>

namespace yk::exec {

using worker_id_t = unsigned;

class worker_pool {
public:
  worker_pool() {  //
    worker_limit_ = static_cast<int>(std::max(2u, std::thread::hardware_concurrency()));
  }

  ~worker_pool() {  //
    halt_and_clear();
  }

  void set_worker_limit(int worker_limit) {
    if (worker_limit < 2) {
      throw std::invalid_argument{"worker limit must be >= 2"};
    }
    worker_limit_ = worker_limit;
  }

  [[nodiscard]]
  int get_worker_limit() const noexcept {
    return worker_limit_;
  }

  [[nodiscard]]
  int launched_worker_count() const noexcept {
    return static_cast<int>(threads_.size());
  }

  [[nodiscard]]
  std::stop_token stop_token() const noexcept {
    return stop_source_.get_token();
  }

  [[nodiscard]]
  bool stop_requested() const noexcept {
    return stop_source_.stop_requested();
  }

  void halt() {
    if (!stop_source_.stop_requested()) {
      stop_source_.request_stop();
    }
    stop_source_ = {};
  }

  void halt_and_clear() {
    halt();

    for (auto& thread : threads_) {
      thread.join();
    }
    threads_.clear();
  }

  template <class F>
  void launch(F&& f) {
    static_assert(std::invocable<F, worker_id_t, std::stop_token>);

    threads_.emplace_back([this, id = static_cast<worker_id_t>(threads_.size()), f = std::forward<F>(f)]() mutable {
      try {
        f(id, stop_source_.get_token());

      } catch (const yk::interrupt_exception&) {
      }
    });
  }

  template <class F>
  void launch_rest(F&& f) {
    const auto remaining = worker_limit_ - launched_worker_count();

    for (int i = 0; i < remaining; ++i) {
      launch(f);
    }
  }

private:
  int worker_limit_ = 2;
  std::vector<std::thread> threads_;
  std::stop_source stop_source_;
};

}  // namespace yk::exec

#endif
