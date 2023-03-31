#include "blocking_queue.h"

namespace idle {
namespace concurrency {
template <typename T>
BlockingQueue<T>::BlockingQueue(int cap) : capacity_(cap) {}

template <typename T> BlockingQueue<T>::~BlockingQueue() {}

template <typename T> void BlockingQueue<T>::push(T value) {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.size() == capacity_) {
    not_full_.wait(lock);
  }
  queue_.push(std::move(value));
  not_empty_.notify_all();
}

template <typename T> T BlockingQueue<T>::pop() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.empty()) {
    not_empty_.wait(lock);
  }
  T value = std::move(queue_.front());
  queue_.pop();
  not_full_.notify_all();
  return value;
}
} // namespace concurrency
} // namespace idle