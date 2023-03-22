#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <stdint.h>
#include <queue>

namespace idle
{
  namespace concurrency
  {
    class Task
    {
    public:
      Task() {}
      Task(int64_t id, int64_t expireTime, std::function<void()> func, bool isBackground) : id_(id), expire_time_(expireTime), func_(func), is_background_(isBackground) {}

    private:
      int64_t id_;
      int64_t expire_time_;
      bool is_background_;
      std::function<void()> func_;
      bool operator<(const Task &task) const
      {
        if (expire_time_ != task.expire_time_)
        {
          return expire_time_ > task.expire_time_;
        }
        else
        {
          return id_ > task.id_;
        }
      }
    };

    // Executor is a class that manage a number of threads, which can be used to execute
    // 1. background tasks
    // 2. delay tasks
    // 3. cron tasks
    class Executor
    {
    public:
      Executor();
      ~Executor();
      void Start();

    private:
      using TaskQueue = std::priority_queue<Task>;
      using TaskMap = std::map<int64_t, Task>;

      int32_t thread_num_;
      volatile int pending_num_;
      std::mutex mu_;
      std::condition_variable cv_;
      bool stop_;

      TaskQueue normal_;
      TaskQueue background_;
    };
  }
}