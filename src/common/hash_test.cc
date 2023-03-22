#include <gtest/gtest.h>

#include "hash.h"

using namespace idle::common;

// Unit test 1
TEST(HashTest, Basic)
{
  uint32_t seed = 12345;
  std::string_view str = "hello world";
  uint32_t expected_hash = 3590098352;
  EXPECT_EQ(hash(str, str.size(), seed), expected_hash);
}

// Unit test 2
TEST(HashTest, EmptyString)
{
  uint32_t seed = 54321;
  std::string_view str = "";
  uint32_t expected_hash = seed;
  GTEST_LOG_(INFO) << hash(str, str.size(), seed);
  EXPECT_EQ(hash(str, str.size(), seed), expected_hash);
}