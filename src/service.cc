#include "service.h"
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
namespace idle {
namespace service {
void Init(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(ERROR) << "asyncd::bootstrap"
             << "version=0.0.1";
}

template <typename T>
Service<T>::Service(std::string_view addr, std::string_view name, T service)
    : service_(service), addr_(addr), name_(name) {}

template <typename T> Service<T>::~Service<T>() {}

template <typename T> void Service<T>::Wait() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(addr_.data(), grpc::InsecureServerCredentials());
  builder.RegisterService(&service_);
  builder.BuildAndStart()->Wait();
}
} // namespace service
} // namespace idle