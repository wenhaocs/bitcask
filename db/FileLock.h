#ifndef DB_FILELOCK_H_
#define DB_FILELOCK_H_

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"

namespace bitcask {
// Identifies a locked file.
class FileLock {
 public:
  FileLock() = default;

  FileLock(const FileLock&) = delete;
  FileLock& operator=(const FileLock&) = delete;

  ~FileLock() = default;

  StatusOr<bool> tryLock();
};

}  // namespace bitcask

#endif  // DB_FILELOCK_H_
