#include "executor.h"

namespace idle
{
  namespace concurrency
  {
    Executor::Executor() : thread_num_(10), pending_num_(0) {}

    Executor::~Executor() {}

    void Executor::Start() {}
  }

}
