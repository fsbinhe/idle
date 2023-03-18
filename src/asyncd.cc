#include <glog/logging.h>
#include <gflags/gflags.h>

#include <async/v1/async.pb.h>
#include <async/v1/async.grpc.pb.h>

#include <raft/v1/raft.pb.h>
#include <raft/v1/raft.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include <iostream>
#include "leveldb/db.h"

#include <folly/dynamic.h>

class RaftServiceImpl final : public raft::v1::Raft::Service
{
public:
  RaftServiceImpl() {}

  virtual ::grpc::Status VoteRequest(::grpc::ServerContext *ctx, const raft::v1::VoteRequest *req, raft::v1::VoteResponse *resp)
  {
    return grpc::Status::OK;
  }
};

class AsyncServiceImpl final : public async::v1::Async::Service
{
public:
  AsyncServiceImpl()
  {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/asyncddb", &db);
    assert(status.ok());
    LOG(ERROR) << "asyncd: storage initiliased";
  }

  virtual ::grpc::Status CreateTask(::grpc::ServerContext *ctx, const async::v1::CreateTaskRequest *req, async::v1::CreateTaskResponse *resp)
  {
    LOG(ERROR) << "asyncd::CreateTask";
    resp->set_id("10087");
    resp->set_error_code(0);
    resp->set_message("OK");
    return grpc::Status::OK;
  }

private:
  leveldb::DB *db;
};

int main(int argc, char *argv[])
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(ERROR) << "asyncd::bootstrap"
             << "version=0.0.1";
  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
  AsyncServiceImpl service;
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};

  folly::dynamic array = folly::dynamic::array("one", 2, 3.0);

  server->Wait();
  return 0;
}