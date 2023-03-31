#include <gflags/gflags.h>
#include <glog/logging.h>

#include <async/v1/async.grpc.pb.h>
#include <async/v1/async.pb.h>

#include <raft/v1/raft.grpc.pb.h>
#include <raft/v1/raft.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include "leveldb/db.h"
#include "service.h"
#include <iostream>

#include <folly/dynamic.h>

class RaftServiceImpl final : public raft::v1::Raft::Service {
public:
  RaftServiceImpl() {}

  virtual ::grpc::Status VoteRequest(::grpc::ServerContext *ctx,
                                     const raft::v1::VoteRequest *req,
                                     raft::v1::VoteResponse *resp) {
    return grpc::Status::OK;
  }
};

class AsyncServiceImpl final : public async::v1::Async::Service {
public:
  AsyncServiceImpl() {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/asyncddb", &db);
    assert(status.ok());
    LOG(ERROR) << "asyncd: storage initiliased";
  }

  virtual ::grpc::Status CreateTask(::grpc::ServerContext *ctx,
                                    const async::v1::CreateTaskRequest *req,
                                    async::v1::CreateTaskResponse *resp) {
    LOG(ERROR) << "asyncd::CreateTask";
    resp->set_id("10087");
    resp->set_error_code(0);
    resp->set_message("OK");
    return grpc::Status::OK;
  }

private:
  leveldb::DB *db;
};

int main(int argc, char *argv[]) {
  idle::service::Init(argc, argv);
  AsyncServiceImpl service;
  idle::service::Service<AsyncServiceImpl> asyncd("0.0.0.0:50051", "asyncd",
                                                  service);
  asyncd.Wait();
  return 0;
}