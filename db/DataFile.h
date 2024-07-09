#ifndef DB_DATAFILE_H_
#define DB_DATAFILE_H_

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"
#include "db/LogRecord.h"

namespace bitcask {

class DataFile {
 public:
  DataFile() = default;

  DataFile(const std::string dirPath, const uint32_t fileId, bool readOnly = false);

  Status openDataFile();

  Status closeDataFile();

  // read a LogRecord from datafile
  StatusOr<std::unique_ptr<LogRecord>> readLogRecord(int64_t pos);

  // encode the log and write the buffer to datafile
  // return the position of this log record
  StatusOr<int64_t> writeLogRecord(std::unique_ptr<LogRecord> log);

  // force the filesystem to sync all writes from buffer cache to disk
  Status flush();

  DataFile& operator=(const DataFile&) = delete;

  ~DataFile() {
    if (fd_ != -1) {
      close(fd_);
      fd_ = -1;
    }
  }

 private:
  Status readNBytes(int64_t offset, int64_t size, char* buf);

  uint32_t fileId_{0};
  int64_t curWriteOffset_{0};
  std::string fileName_;
  bool readOnly_{false};
  // OS fd when it's open
  int fd_{-1};
};

}  // namespace bitcask

#endif  // DB_DATAFILE_H_
