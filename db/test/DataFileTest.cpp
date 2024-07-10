#include <gtest/gtest.h>

#include <filesystem>

#include "db/DataFile.h"

namespace bitcask {
class DataFileTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // FLAGS_v = 4;
    std::filesystem::create_directories("/tmp/DataFileTest");
  }

  void TearDown() override {
    std::filesystem::remove_all("/tmp/DataFileTest");
  }
};

TEST_F(DataFileTest, SimpleRWTest) {
  std::string dir = "/tmp/DataFileTest/SimpleTest";
  std::filesystem::create_directories(dir);
  auto dataFile = std::make_unique<DataFile>(dir, 1, false);
  auto status = dataFile->openDataFile();
  EXPECT_TRUE(status.ok());

  // write data 1
  int32_t key1 = 111;
  std::string value1 = "test_value1";
  auto record1 = std::make_unique<LogRecord>(key1, value1, LogType::WRITE);

  auto writeRet = dataFile->writeLogRecord(std::move(record1));
  EXPECT_TRUE(writeRet.ok());
  auto pos1 = std::move(writeRet).value();

  // close dataFile and reopen
  status = dataFile->closeDataFile();
  dataFile.reset();
  EXPECT_TRUE(status.ok());
  dataFile = std::make_unique<DataFile>(dir, 1, false);
  status = dataFile->openDataFile();
  EXPECT_TRUE(status.ok());

  // write data 2
  int32_t key2 = 222;
  std::string value2 = "test_value2";
  auto record2 = std::make_unique<LogRecord>(key2, value2, LogType::WRITE);

  writeRet = dataFile->writeLogRecord(std::move(record2));
  EXPECT_TRUE(writeRet.ok());
  auto pos2 = std::move(writeRet).value();

  // read data 1
  auto readRet = dataFile->readLogRecord(pos1);
  ASSERT_TRUE(readRet.ok());
  auto retrievedLog1 = std::move(readRet.value());
  EXPECT_EQ(retrievedLog1->getKey(), key1);
  EXPECT_EQ(retrievedLog1->getValue(), value1);

  // read data 2
  readRet = dataFile->readLogRecord(pos2);
  ASSERT_TRUE(readRet.ok());
  auto retrievedLog2 = std::move(readRet.value());
  EXPECT_EQ(retrievedLog2->getKey(), key2);
  EXPECT_EQ(retrievedLog2->getValue(), value2);

  status = dataFile->closeDataFile();
  dataFile.reset();
  EXPECT_TRUE(status.ok());
}

// Main function for running all tests
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}  // namespace bitcask