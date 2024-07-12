#include <gtest/gtest.h>

#include "db/HashIndex.h"
#include "utils/WallClock.h"

namespace bitcask {

class HashMapIndexTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(HashMapIndexTest, SimpleTest) {
  auto index = std::make_unique<HashIndex>(128);
  KeyType key = 1234;
  auto logPos = std::make_shared<LogPos>(1, 10, 0, time::WallClock::fastNowInMicroSec());
  auto status = index->put(key, logPos);
  EXPECT_TRUE(status.ok());

  // get
  auto ret = index->get(key);
  ASSERT_TRUE(ret.ok());
  auto retrievedLogPos = std::move(ret).value();
  EXPECT_EQ(retrievedLogPos->fileId_, logPos->fileId_);
  EXPECT_EQ(retrievedLogPos->valueSize_, logPos->valueSize_);
  EXPECT_EQ(retrievedLogPos->pos_, logPos->pos_);
  EXPECT_EQ(retrievedLogPos->tstamp_, logPos->tstamp_);

  // remove
  status = index->remove(key);
  EXPECT_TRUE(status.ok());

  // get non-existing key
  ret = index->get(key);
  EXPECT_FALSE(ret.ok());
  EXPECT_EQ(ret.status().message(), "Key not found");
}

}  // namespace bitcask

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
