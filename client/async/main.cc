#include <gflags/gflags.h>
#include <glog/logging.h>

#include <async/v1/async.grpc.pb.h>
#include <async/v1/async.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>
#include <iostream>

int main(int argc, char *argv[]) {

  async::v1::CreateTaskRequest createTaskReq;
  async::v1::CreateTaskResponse createTaskResp;
  createTaskReq.set_handler_id(100);
  createTaskReq.set_body("name: hebin");
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  std::unique_ptr<async::v1::Async::Stub> stub =
      async::v1::Async::NewStub(channel);
  grpc::ClientContext context;
  grpc::Status status =
      stub->CreateTask(&context, createTaskReq, &createTaskResp);
  std::cout << "status=" << status.error_code()
            << ",message=" << status.error_details() << std::endl;
  return 0;
}