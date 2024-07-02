#include "bitcask/Base.h"
#include "bitcask/DB.h"
#include "bitcask/Options.h"

int main(int, char **) {
  LOG(INFO) << "test";
  // FLOG_INFO("This is an info message with number: {}", 42);

  bitcask::Options options;
  auto dbStatus = bitcask::DB::open("test", options);
  if (dbStatus.ok()) {
    return -1;
  }
  auto db = std::move(dbStatus).value();

  return 0;
}
