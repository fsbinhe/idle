#include <cstdlib>
#include <gtest/gtest.h>

#include "raft_storage.h"

using namespace idle::distributed;

// ./src/raft/raft_storage_test --logtostderr=1 --minloglevel=0 | grep "INFO"

TEST(RaftStorageTest, Basic) {
  LogDB *db;
  Option opt;
  LogDB::Open("./testdb", opt, &db);
  ASSERT_TRUE(db != nullptr);

  db->Write(0, "hello world");
  // auto s = LogDB::DestroyDB("./testdb");
  // ASSERT_TRUE(s == kOK);
}