#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
namespace idle {
namespace concurrency {
template <typename T> class BlockingQueue {
public:
  explicit BlockingQueue(int cap);
  ~BlockingQueue();

  void push(T value);

  T pop();

private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  size_t capacity_;
};
} // namespace concurrency
} // namespace idle