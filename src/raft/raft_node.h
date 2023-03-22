#pragma once

#include <raft/v1/raft.pb.h>
#include <raft/v1/raft.grpc.pb.h>
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

namespace idle
{
  namespace distributed
  {
    class RaftNode : public raft::v1::Raft::Service
    {
    public:
      RaftNode();
      ~RaftNode();

      virtual ::grpc::Status VoteRequest(::grpc::ServerContext *ctx, const raft::v1::VoteRequest *req, raft::v1::VoteResponse *resp);

      virtual ::grpc::Status AppendEntries(::grpc::ServerContext *ctx, const raft::v1::AppendEntriesRequest *req, raft::v1::AppendEntriesResponse *resp);

    private:
      // members_ are all nodes in current raft cluster.
      std::vector<std::string> members_;

      // self_ is this node.
      std::string self_;

      int64_t current_term_;
      std::string voted_for_;
    };
  }
}