#include <glog/logging.h>
#include <gflags/gflags.h>

#include <async/v1/async.pb.h>
#include <async/v1/async.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

class AsyncServiceImpl final : public async::v1::Async::Service
{
public:
  virtual ::grpc::Status Echo(::grpc::ServerContext *ctx, const async::v1::EchoRequest *req, async::v1::EchoResponse *resp)
  {
    resp->set_id(req->id());
    return grpc::Status::OK;
  }
};

int main(int argc, char *argv[])
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
  AsyncServiceImpl service;
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
  server->Wait();
  return 0;
}