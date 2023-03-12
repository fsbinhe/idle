#include <glog/logging.h>
#include <gflags/gflags.h>

#include <async/v1/async.pb.h>
#include <async/v1/async.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/create_channel.h>
#include <iostream>

int main(int argc, char *argv[])
{
  async::v1::EchoRequest req;
  async::v1::EchoResponse resp;
  req.set_id(10086);

  auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
  std::unique_ptr<async::v1::Async::Stub> stub = async::v1::Async::NewStub(channel);
  grpc::ClientContext context;
  grpc::Status status = stub->Echo(&context, req, &resp);

  std::cout << "response id=" << resp.id() << std::endl;
  return 0;
}