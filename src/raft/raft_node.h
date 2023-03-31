#pragma once

#include "raft_storage.h"
#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <mutex>
#include <raft/v1/raft.grpc.pb.h>
#include <raft/v1/raft.pb.h>

namespace idle {
namespace distributed {
class RaftNode : public raft::v1::Raft::Service {
public:
  RaftNode() = default;
  RaftNode(const std::string &raft_nodes, int node_index, int election_timeout,
           const std::string &db_path);
  ~RaftNode();

  virtual ::grpc::Status VoteRequest(::grpc::ServerContext *ctx,
                                     const raft::v1::VoteRequest *req,
                                     raft::v1::VoteResponse *resp);

  virtual ::grpc::Status
  AppendEntries(::grpc::ServerContext *ctx,
                const raft::v1::AppendEntriesRequest *req,
                raft::v1::AppendEntriesResponse *resp);

public:
  bool GetLeader(std::string *leader);
  void AppendLog(const std::string &log, std::function<void(bool)> callback);
  bool AppendLog(const std::string &log, int timeout_ms = 10000);
  void Init(std::function<void(const std::string &log)> callback,
            std::function<void(int32_t, std::string *)> snapshot_callback);

private:
  bool StoreContext(const std::string &context, int64_t value);
  bool StoreContext(const std::string &context, const std::string &value);
  raft::v1::Raft::Stub GetStub();

  std::string Index2LogKey(int64_t index);
  void LoadStorage(const std::string &db_path);
  bool CancelElection();
  bool ResetElection();
  void ReplicateLogForNode(uint32_t id);
  void ReplicateLogWorker(uint32_t id);
  void Election();
  bool CheckTerm(int64_t term);
  void ElectionCallback(const raft::v1::VoteRequest *request,
                        raft::v1::VoteResponse *response, bool faild, int error,
                        const std::string &node_addr);
  bool StoreLog(int64_t term, int64_t index, const std::string &log,
                raft::v1::LogType type = raft::v1::kUserLog);
  void ApplyLog();
  std::string LoadVotedFor();
  void SetVoteFor(const std::string &voteFor);
  int64_t LoadCurrentTerm();
  void SetCurrentTerm(int64_t);
  void SetLastApplied(int64_t index);
  int64_t GetLastApplied(int64_t index);

private:
  // members_ are all nodes in current raft cluster.
  std::vector<std::string> members_;
  // self_ is this node.
  std::string self_;
  int64_t current_term_;
  std::string voted_for_;
  LogDB *log_db_;
  int64_t log_index_;
  int64_t log_term_;
  int64_t commit_index_;
  int64_t last_applied_;
  bool applying_;
  bool node_stop_;
  struct FollowerContext {
    int64_t next_index_;
    int64_t match_indext_;
    concurrency::Executor *executor_;
    std::condition_variable condition;
    FollowerContext(std::mutex *mu) {}
  };
  std::vector<FollowerContext *> follower_contexts_;
  std::mutex mu_;
  concurrency::Executor *executor_;
  raft::v1::Raft::Stub *stub_;
  std::set<std::string> voted_;
  std::string leader_;
  int64_t election_taskid_;
  int64_t election_timeout_;

  std::function<void(const std::string &log)> log_callback_;
  std::map<int64_t, std::function<void(bool)>> callback_map_;

  enum NodeState {
    kFollower = 0,
    kCandidate = 1,
    kLeader = 2,
  };

  NodeState state_;
};
} // namespace distributed
} // namespace idle