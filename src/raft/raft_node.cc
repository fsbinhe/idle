#include "raft_node.h"

namespace idle
{
  namespace distributed
  {
    RaftNode::RaftNode() {}

    ::grpc::Status RaftNode::VoteRequest(::grpc::ServerContext *ctx, const raft::v1::VoteRequest *req, raft::v1::VoteResponse *resp)
    {
      return grpc::Status::OK;
    }

    ::grpc::Status RaftNode::AppendEntries(::grpc::ServerContext *ctx, const raft::v1::AppendEntriesRequest *req, raft::v1::AppendEntriesResponse *resp)
    {
      return grpc::Status::OK;
    }

    RaftNode::~RaftNode() {}
  }
}