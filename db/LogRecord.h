#ifndef DB_LOGRECORD_H_
#define DB_LOGRECORD_H_

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"

namespace bitcask {

// Structure of log record in data file
// crc | tstamp | LogType | keySize | valueSize | key | value
class LogRecord {
 public:
  enum class LogType : uint8_t {
    WRITE = 0,
    DELETE = 1,
  };

  LogRecord() = default;

  LogRecord(const LogRecord&) = delete;
  LogRecord& operator=(const LogRecord&) = delete;

  LogRecord(const int32_t& key, const std::string& value, const LogType logType);


  ~LogRecord() = default;

 private:
  int32_t key_;
  uint8_t keySize_{4};
  std::string value_;
  uint16_t valueSize_{0};
  int64_t tstamp_;
  uint32_t crc_;
  LogType logType_;

  // store encoded str
  std::string buf_;

  static constexpr int kMaxValueSize = 4096;
};

}  // namespace bitcask

#endif  // DB_LOGRECORD_H_
