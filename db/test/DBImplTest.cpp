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
    auto dbPtr = std::move(ret).value();
    // dbPtr->put(1234, "test_value");

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
    auto dbPtr = std::move(ret).value();
  }

  {  // Open db with existing files in read only mode
    std::string dbname = "/tmp/DBImplTest/SimpleTest";
    bitcask::Options options;
    options.readOnly = true;
    auto ret = DB::open(dbname, options);
    ASSERT_TRUE(ret.ok());
    auto dbPtr = std::move(ret).value();
  }
}

}  // namespace bitcask

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
