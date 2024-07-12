#include <gtest/gtest.h>

#include "db/DBImpl.h"

namespace bitcask {

class DBImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    FLAGS_v = 2;
    std::filesystem::create_directories("/tmp/DBImplTest");
  }

  void TearDown() override {
    std::filesystem::remove_all("/tmp/DBImplTest");
  }
};

TEST_F(DBImplTest, OpenTest) {
  {  // Open a new db in read only mode
    std::string dbname = "/tmp/DBImplTest/SimpleTest";
    bitcask::Options options;
    options.readOnly = true;
    auto ret = DB::open(dbname, options);
    ASSERT_FALSE(ret.ok());
  }

  {  // Open a new db in rw mode
    std::string dbname = "/tmp/DBImplTest/SimpleTest";
    bitcask::Options options;
    options.readOnly = false;
    auto ret = DB::open(dbname, options);
    ASSERT_TRUE(ret.ok());
    auto db = std::move(ret).value();
    // db->put(1234, "test_value");

    // open the same db in rw mode again
    ret = DB::open(dbname, options);
    ASSERT_FALSE(ret.ok());
  }

  {  // Open db with existing files in rw mode
    std::string dbname = "/tmp/DBImplTest/SimpleTest";
    bitcask::Options options;
    options.readOnly = false;
    auto ret = DB::open(dbname, options);
    ASSERT_TRUE(ret.ok());
    auto db = std::move(ret).value();
  }

  {  // Open db with existing files in read only mode
    std::string dbname = "/tmp/DBImplTest/SimpleTest";
    bitcask::Options options;
    options.readOnly = true;
    auto ret = DB::open(dbname, options);
    ASSERT_TRUE(ret.ok());
    auto db = std::move(ret).value();
  }
}

TEST_F(DBImplTest, SimplePutGetTest) {
  std::string dbname = "/tmp/DBImplTest/PutGetTest";
  bitcask::Options options;
  options.readOnly = false;
  auto ret = DB::open(dbname, options);
  ASSERT_TRUE(ret.ok());
  auto db = std::move(ret).value();

  // Put a key-value pair
  KeyType key = 1234;
  std::string value = "value1";
  auto status = db->put(key, value);
  ASSERT_TRUE(status.ok());

  // Get the value
  auto getRet = db->get(key);
  ASSERT_TRUE(getRet.ok());
  ASSERT_EQ(getRet.value(), value);
}

TEST_F(DBImplTest, DeleteTest) {
  std::string dbname = "/tmp/DBImplTest/DeleteTest";
  bitcask::Options options;
  options.readOnly = false;
  auto ret = DB::open(dbname, options);
  ASSERT_TRUE(ret.ok());
  auto db = std::move(ret).value();

  // Put a key-value pair
  KeyType key = 1234;
  std::string value = "value1";
  auto status = db->put(key, value);
  ASSERT_TRUE(status.ok());

  // Delete the key
  auto deleteStatus = db->deleteKey(key);
  ASSERT_TRUE(deleteStatus.ok());

  // Try to get the deleted key
  auto getRet = db->get(key);
  ASSERT_FALSE(getRet.ok());
  ASSERT_EQ(getRet.status().code(), Status::Code::kNotFound);
}

TEST_F(DBImplTest, ListKeysTest) {
  std::string dbname = "/tmp/DBImplTest/ListKeysTest";
  bitcask::Options options;
  options.readOnly = false;
  auto ret = DB::open(dbname, options);
  ASSERT_TRUE(ret.ok());
  auto db = std::move(ret).value();

  // Put some key-value pairs
  KeyType key1 = 1;
  std::string value1 = "value1";
  auto putStatus = db->put(key1, value1);
  ASSERT_TRUE(putStatus.ok());

  KeyType key2 = 2;
  std::string value2 = "value2";
  putStatus = db->put(key2, value2);
  ASSERT_TRUE(putStatus.ok());

  // List keys
  auto listRet = db->listKeys();
  ASSERT_TRUE(listRet.ok());
  auto keys = listRet.value();
  ASSERT_EQ(keys.size(), 2);
  ASSERT_NE(std::find(keys.begin(), keys.end(), key1), keys.end());
  ASSERT_NE(std::find(keys.begin(), keys.end(), key2), keys.end());
}

TEST_F(DBImplTest, PutExceedingFileLimitTest) {
  std::string dbname = "/tmp/DBImplTest/PutExceedingFileLimitTest";
  bitcask::Options options;
  options.maxFileSize = 128;  // 128B max file size
  options.readOnly = false;
  auto ret = DB::open(dbname, options);
  ASSERT_TRUE(ret.ok());
  auto db = std::move(ret).value();

  // Log header is 16B. So each LogRecord is 28B. Each data file should store 4 LogRecords. A total
  // of 25 data files should be created.
  for (auto i = 0; i < 100; i++) {
    KeyType key = reinterpret_cast<KeyType>(i);
    std::ostringstream ss;
    ss << std::setw(8) << std::setfill('0') << i;
    std::string value = ss.str();
    auto status = db->put(key, value);
    ASSERT_TRUE(status.ok());
  }

  auto dbPtr = dynamic_cast<DBImpl*>(db.get());
  EXPECT_EQ(25, dbPtr->activeFileId_);
  EXPECT_EQ(25, dbPtr->allFileIds_.size());

  db->close();
  delete (db.release());

  // Open the db again, check the data files and read data.
  ret = DB::open(dbname, options);
  ASSERT_TRUE(ret.ok());
  db = std::move(ret).value();

  dbPtr = dynamic_cast<DBImpl*>(db.get());
  EXPECT_EQ(25, dbPtr->activeFileId_);
  EXPECT_EQ(25, dbPtr->allFileIds_.size());

  for (auto i = 0; i < 100; i++) {
    KeyType key = reinterpret_cast<KeyType>(i);
    std::ostringstream ss;
    ss << std::setw(8) << std::setfill('0') << i;
    std::string expectedValue = ss.str();
    auto getRet = db->get(key);
    EXPECT_TRUE(getRet.ok());
    EXPECT_EQ(getRet.value(), expectedValue);
  }

  db->close();
}

TEST_F(DBImplTest, ConcurrentReadWriteTest) {
  std::string dbname = "/tmp/DBImplTest/ConcurrentReadWriteTest";
  bitcask::Options options;
  options.maxFileSize = 1024;  // 1KB max file size
  options.readOnly = false;
  auto ret = DB::open(dbname, options);
  ASSERT_TRUE(ret.ok());
  auto db = std::move(ret).value();

  const int numThreads = 100;
  const int numOperations = 100;

  // Thread 0 will write keys 0-99, values value_0_0-value_0_99
  // Thread 1 will write keys 100-199, values value_1_0-value_1_99
  auto rwFunc = [&db](int threadId) {
    for (int i = 0; i < numOperations; ++i) {
      KeyType key = reinterpret_cast<KeyType>(threadId * numOperations + i);
      std::ostringstream ss;
      auto value = fmt::format("value_{}_{}", threadId, i);
      auto status = db->put(key, value);
      ASSERT_TRUE(status.ok());

      auto getRet = db->get(key);
      ASSERT_TRUE(getRet.ok());
      EXPECT_EQ(getRet.value(), value);
    }
  };

  // Create threads for writing
  std::vector<std::thread> threads;
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(rwFunc, i);
  }

  // Join all threads
  for (auto& t : threads) {
    t.join();
  }

  db->close();
}

TEST_F(DBImplTest, IteratorTest) {
  std::string dbname = "/tmp/DBImplTest/IteratorTest";
  bitcask::Options options;
  options.maxFileSize = 1024;  // 1KB max file size
  options.readOnly = false;
  auto ret = DB::open(dbname, options);
  ASSERT_TRUE(ret.ok());
  auto db = std::move(ret).value();

  // Add some key-value pairs
  for (int i = 0; i < 100; ++i) {
    KeyType key = reinterpret_cast<KeyType>(i);
    std::string value = "value_" + std::to_string(i);
    auto status = db->put(key, value);
    ASSERT_TRUE(status.ok());
  }

  // Create an iterator and iterate through the keys
  std::function<void(const KeyType&, const std::string&)> func = [](const KeyType& key,
                                                                    const std::string& value) {
    std::string expectedValue = "value_" + std::to_string(key);
    EXPECT_EQ(value, expectedValue);
  };

  db->fold(std::move(func));

  db->close();
}

}  // namespace bitcask

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
