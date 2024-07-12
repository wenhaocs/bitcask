#include "bitcask/Base.h"
#include "bitcask/DB.h"
#include "bitcask/Options.h"

int main(int, char **) {
  LOG(INFO) << "test";
  // FLOG_INFO("This is an info message with number: {}", 42);

  bitcask::Options options;
  std::string dbPath = "test";
  auto dbStatus = bitcask::DB::open(dbPath, options);
  if (dbStatus.ok()) {
    return -1;
  }
  auto db = std::move(dbStatus).value();

  bitcask::Status status = db->put("testKey", "value");

  auto valueRet = db->get("testKey");

  auto allKeysRet = db->listKeys();

  status = db->merge(dbPath);

  status = db->sync();

  status = db->deleteKey("testKey");

  status = db->close();
  return 0;
}
