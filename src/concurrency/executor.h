#pragma once

#include "blocking_queue.h"
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <stdint.h>
#include <thread>

namespace idle {
namespace concurrency {
class Task {
public:
  Task() {}
  Task(int64_t id, int64_t expireTime, std::function<void()> func,
       bool isBackground)
      : id_(id), expire_time_(expireTime), func_(func),
        is_background_(isBackground) {}
  void operator()() { func_(); }

private:
  int64_t id_;
  int64_t expire_time_;
  bool is_background_;
  std::function<void()> func_;
  bool operator<(const Task &task) const {
    if (expire_time_ != task.expire_time_) {
      return expire_time_ > task.expire_time_;
    } else {
      return id_ > task.id_;
    }
  }
};

// Executor is a class that manage a number of threads, which can be used to
// execute
// 1. background tasks
// 2. delay tasks
// 3. cron tasks
class Executor {
public:
  Executor();
  ~Executor();

  void Submit(Task &&task);

private:
  using TaskQueue = BlockingQueue<Task>;
  using TaskMap = std::map<int64_t, Task>;

  int32_t thread_num_;
  volatile int pending_num_;
  std::mutex mu_;
  bool stop_;

  TaskQueue *normal_;
  // TaskQueue background_;

  std::vector<std::thread> threads_;
};
} // namespace concurrency
} // namespace idle