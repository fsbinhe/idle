#include "raft_node.h"
#include "raft/v1/raft.pb.h"
#include "raft_storage.h"
#include <future>
#include <glog/logging.h>
#include <grpcpp/client_context.h>
#include <mutex>
#include <utility>
namespace idle {
namespace distributed {

RaftNode::RaftNode(const std::string &raft_nodes, int node_index,
                   int election_timeout, const std::string &db_path)
    : current_term_(0), log_index_(0), log_term_(0), commit_index_(0),
      last_applied_(0), applying_(false), node_stop_(false),
      election_taskid_(-1), election_timeout_(election_timeout),
      state_(kFollower) {}

::grpc::Status RaftNode::VoteRequest(::grpc::ServerContext *ctx,
                                     const raft::v1::VoteRequest *req,
                                     raft::v1::VoteResponse *resp) {
  return grpc::Status::OK;
}

RaftNode::~RaftNode() {}
::grpc::Status
RaftNode::AppendEntries(::grpc::ServerContext *ctx,
                        const raft::v1::AppendEntriesRequest *req,
                        raft::v1::AppendEntriesResponse *resp) {
  return grpc::Status::OK;
}

bool RaftNode::GetLeader(std::string *leader) {
  if (state_ != kLeader) {
    if (leader_ != "" && leader_ != self_ && leader) {
      *leader = leader_;
    }
    return false;
  }
  if (last_applied_ < log_index_) {
    LOG(INFO) << "[raftd] Leader recover log log_index = " << log_index_
              << " commit_index = " << commit_index_
              << " last_applied = " << last_applied_;
  }
  return true;
}

void RaftNode::AppendLog(const std::string &log,
                         std::function<void(bool)> callback) {
  std::lock_guard<std::mutex> lock(mu_);
  int64_t index = log_index_ + 1;
  if (!StoreLog(current_term_, index, log)) {
    // executor_->Submit(callback);
    return;
  }
  callback_map_.insert(std::make_pair(index, callback));
  log_index_++;
  for (uint32_t i = 0; i < members_.size(); i++) {
    if (follower_contexts_[i]) {
      follower_contexts_[i]->condition.notify_all();
    }
  }
}

bool RaftNode::AppendLog(const std::string &log, int timeout_ms) {
  int64_t index = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    index = log_index_ + 1;
    if (!StoreLog(current_term_, index, log)) {
      return false;
    }
    log_index_++;
  }

  for (uint32_t i = 0; i < members_.size(); i++) {
    if (follower_contexts_[i]) {
      follower_contexts_[i]->condition.notify_all();
    }
  }
  for (int i = 0; i < timeout_ms; i++) {
    usleep(1);
    if (commit_index_ >= index) {
      return true;
    }
  }
  return false;
}

void RaftNode::Election() {
  std::lock_guard<std::mutex> lock(mu_);
  if (state_ == kLeader) {
    election_taskid_ = -1;
    return;
  }

  voted_.clear();
  current_term_++;
  state_ = kCandidate;
  voted_for_ = self_;
  if (!StoreContext("current_term", current_term_) ||
      !StoreContext("voted_for", voted_for_)) {
    LOG(FATAL) << "[raftd] Election store term and vote_for failed voted = "
               << voted_for_ << " current_term = " << current_term_;
    voted_.insert(self_);
    LOG(INFO) << "[raftd] Election start to elect current_term = "
              << current_term_ << " log_index = " << log_index_
              << " log_term = " << log_term_;
    for (int i = 0; i < members_.size(); i++) {
      if (members_[i] == self_) {
        continue;
      }
      LOG(INFO) << "[raftd] Election send VoteRequest to member = "
                << members_[i].c_str();
      raft::v1::VoteRequest req;
      req.set_term(current_term_);
      req.set_candidate(self_);
      req.set_last_log_index(log_index_);
      req.set_last_log_term(log_term_);
      raft::v1::VoteResponse *resp = new raft::v1::VoteResponse();
      auto stub = GetStub();
      grpc::ClientContext context;
      auto stutus = stub.Vote(&context, req, resp);
    }
  }

  // election_taskid_ = executor_->DelayTask(election_timeout_ + rand() %
  // election_timeout_, std::bind(&RaftNodeImpl::Election, this));
}

bool RaftNode::CheckTerm(int64_t term) {
  if (term > current_term_) {
    if (state_ == kLeader) {
      LOG(FATAL) << "[raftd] CheckTerm leader change to follower, exit";
    } else {
      LOG(INFO) << "[raftd] CheckTerm change state to follower, reset election";
    }

    current_term_ = term;
    voted_for_ = "";
    if (!StoreContext("current_term", current_term_) ||
        (!StoreContext("voted_for", voted_for_))) {
      LOG(FATAL)
          << "[raftd] CheckTerm store term and vote_for failed voted_for = "
          << voted_for_ << " current_term" << current_term_;
    }
    state_ = kFollower;
    ResetElection();
    return false;
  }
  return true;
}

void RaftNode::LoadStorage(const std::string &db_path) {
  LogDB::Open(db_path, Option(), &log_db_);
  if (log_db_ == nullptr) {
    LOG(FATAL) << "[raftd] LoadStorage failed";
    return;
  }
  StatusCode s = log_db_->ReadMarker("last_applied", &last_applied_);
  if (s != kOK) {
    last_applied_ = 0;
  }
  s = log_db_->ReadMarker("current_term", &current_term_);
  if (s != kOK) {
    current_term_ = 0;
  }
  s = log_db_->ReadMarker("voted_for", &voted_for_);
  if (s != kOK) {
    voted_for_ = "";
  }
  s = log_db_->GetLargestIndex(&log_index_);
  if (s != kOK) {
    log_index_ = 0;
  }
  LOG(INFO) << "[raftd] LoadStorage term = " << current_term_
            << " index = " << log_index_ << " applied = " << last_applied_;
  for (uint32_t i = 0; i < members_.size(); i++) {
    if (members_[i] == self_) {
      follower_contexts_.push_back(nullptr);
      continue;
    }
    FollowerContext *ctx = new FollowerContext(&mu_);
    follower_contexts_.push_back(ctx);
    LOG(INFO) << "[raftd] LoadStorage New follower context member = "
              << members_[i].c_str();
    ctx->next_index_ = log_index_ + 1;
  }
}

void RaftNode::ApplyLog() {
  std::unique_lock<std::mutex> lock(mu_);
  if (applying_ || !log_callback_) {
    return;
  }
  applying_ = true;
  for (int64_t i = last_applied_ + 1; i <= commit_index_; i++) {
    std::string log;
    StatusCode s = log_db_->Read(i, &log);
    if (s != kOK) {
      LOG(FATAL) << "[raftd] Read logdb index = " << i << " failed";
    }
    raft::v1::LogEntry entry;
    bool ret = entry.ParseFromString(log);
    assert(ret);
    if (entry.type() == raft::v1::kUserLog) {
      mu_.unlock();
      LOG(INFO) << "[raftd] callback index = " << entry.index()
                << " str = " << entry.log_data();
      log_callback_(entry.log_data());
      log_callback_(entry.log_data());
      mu_.lock();
    }
    last_applied_ = entry.index();
  }
  LOG(INFO) << "[raftd] Apply to index = " << last_applied_;
  StoreContext("last_applied", last_applied_);
  applying_ = false;
}

void RaftNode::Init(
    std::function<void(const std::string &log)> callback,
    std::function<void(int32_t, std::string *)> /*snapshot_callback*/) {
  log_callback_ = callback;
  ApplyLog();
}
} // namespace distributed
} // namespace idle