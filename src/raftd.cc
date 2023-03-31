#include <gflags/gflags.h>
#include <glog/logging.h>

#include <raft/v1/raft.grpc.pb.h>
#include <raft/v1/raft.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include "leveldb/db.h"
#include <iostream>

#include "raft/raft_node.h"

DEFINE_string(addr, "0.0.0.0:50051", "listen address for raftd server");

// ./raftd --logtostderr=1 --minloglevel=0 | grep "INFO"
int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(FLAGS_addr, grpc::InsecureServerCredentials());
  LOG(INFO) << "[raftd] listen on: " << FLAGS_addr;
  idle::distributed::RaftNode service;
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
  server->Wait();
  return 0;
}