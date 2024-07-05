#ifndef DB_DATAFILE_H_
#define DB_DATAFILE_H_

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"

namespace bitcask {

class DataFile {
 public:
  DataFile() = default;

  DataFile(const FileLock&) = delete;
  DataFile& operator=(const DataFile&) = delete;

  ~DataFile() = default;

 private:
  uint32_t fileId_;
};

}  // namespace bitcask

#endif  // DB_DATAFILE_H_
