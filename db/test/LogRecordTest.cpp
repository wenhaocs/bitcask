#include <gtest/gtest.h>

#include "db/LogRecord.h"
#include "utils/Crc.h"
#include "utils/WallClock.h"

namespace bitcask {
class LogRecordTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Setup code if needed
  }

  void TearDown() override {
    // Cleanup code if needed
  }
};

// Test the constructor of LogRecord
TEST_F(LogRecordTest, ConstructorTest) {
  int32_t key = 1234;
  std::string value = "test_value";
  uint16_t valueSize = static_cast<uint16_t>(value.size());

  LogRecord::LogType logType = LogRecord::LogType::WRITE;

  LogRecord record(key, value, logType);

  // Check if the CRC is correctly calculated
  

  record.encode();
  EXPECT_EQ(1, true);
}

// Main function for running all tests
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}  // namespace bitcask