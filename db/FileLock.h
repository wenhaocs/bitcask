#ifndef DB_FILELOCK_H_
#define DB_FILELOCK_H_

#include "bitcask/Base.h"
#include "bitcask/Status.h"

namespace bitcask {
class FileLock {
 public:
  FileLock() = default;
  explicit FileLock(const std::string& fileName);

  FileLock(const FileLock&) = delete;
  FileLock& operator=(const FileLock&) = delete;

  ~FileLock();

  Status tryLock();
  Status unlock();

 private:
  std::string fileName_;
  int fd_{-1};
};

}  // namespace bitcask

#endif  // DB_FILELOCK_H_
