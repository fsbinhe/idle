#include <cstdlib>
#include <gtest/gtest.h>

#include "raft_storage.h"

using namespace idle::distributed;

TEST(RaftStorageTest, Basic) {
  LogDB *db;
  Option opt;
  LogDB::Open("./testdb", opt, &db);
  ASSERT_TRUE(db != nullptr);
}