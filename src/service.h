#pragma once
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <string>
#include <string_view>
namespace idle {
namespace service {
void Init(int argc, char *argv[]);
template <typename T> class Service {
public:
  Service(std::string_view addr, std::string_view name, T service);
  ~Service();
  void Wait();

private:
  T service_;
  std::string_view addr_;
  std::string_view name_;
};
} // namespace service
} // namespace idle