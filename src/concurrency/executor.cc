#include "executor.h"
#include "blocking_queue.h"

namespace idle {
namespace concurrency {
Executor::Executor()
    : thread_num_(10), pending_num_(0), normal_(new BlockingQueue<Task>(100)) {
  for (int i = 0; i < thread_num_; i++) {
    threads_.emplace_back([this]() {
      while (true) {
        auto task = normal_->pop();
        task();
        if (stop_) {
          break;
        }
      }
    });
  }
}

Executor::~Executor() {
  stop_ = true;
  for (auto &thread : threads_) {
    thread.join();
  }
}

void Executor::Submit(Task &&task) { normal_->push(std::move(task)); }

} // namespace concurrency

} // namespace idle
