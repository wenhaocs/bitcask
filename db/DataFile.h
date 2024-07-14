#ifndef DB_DATAFILE_H_
#define DB_DATAFILE_H_

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"
#include "bitcask/Types.h"
#include "db/LogRecord.h"

namespace bitcask {

class DataFile {
 public:
  DataFile() = default;

  DataFile(const std::string dirPath, const uint32_t fileId, bool readOnly = false);

  Status openDataFile();

  Status closeDataFile();

  // read a LogRecord from datafile
  StatusOr<std::unique_ptr<LogRecord>> readLogRecord(FileOffset pos);

  // read a LogRecord from datafile with knowledge of value size
  StatusOr<std::unique_ptr<LogRecord>> readLogRecord(FileOffset pos, uint16_t valueSize);

  // encode the log and write the buffer to datafile
  // return the position of this log record
  StatusOr<FileOffset> writeLogRecord(std::unique_ptr<LogRecord>&& log);

  // force the filesystem to sync all writes from buffer cache to disk
  Status flush();

  // get the current data file size
  int64_t getCurrentFileSize();

  DataFile& operator=(const DataFile&) = delete;

  ~DataFile() {
    if (fd_ != -1) {
      close(fd_);
      fd_ = -1;
    }
  }

 private:
  Status readNBytes(int64_t offset, int64_t size, char* buf);

  FileID fileId_{0};
  FileOffset curWriteOffset_{0};
  std::string fileName_;
  bool readOnly_{false};

  // OS fd when it's open
  // Race condition:
  // (1) open and anything else. openDataFile is called in db->open() and appendLogRecord.
  // db->open() is always called first, and then read or write, no possibility of race condition.
  // appendLogRecord has a lock to serialize the close, open, and write. No concurrent access to fds
  // during appendLogRecord. (2) close and anything. closeDataFile is called in appendLogRecord
  // only, which is protected by lock. (3) concurrent write: impossible. appendLogRecord is
  // serialized. (4) concurrent read and write. When we read a key, we are reading the underlying
  // datafile via offset. Since the data is written in append only mode, the data at the offset
  // won't be modified. So there is no race condition.
  int fd_{-1};
};

}  // namespace bitcask

#endif  // DB_DATAFILE_H_
