#ifndef DB_LOGRECORD_H_
#define DB_LOGRECORD_H_

#include <gtest/gtest_prod.h>

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"
#include "bitcask/Types.h"

namespace bitcask {

enum class LogType : uint8_t {
  WRITE = 0,
  DELETE = 1,
};

struct LogRecordHeader {
  uint32_t crc_;
  int64_t tstamp_;
  LogType logType_;
  uint8_t keySize_{4};  // fixed 4B key size
  uint16_t valueSize_{0};
};

// there can be padding, so sum individual ones. 16B.
static const size_t kLogHeaderSize =
    sizeof(uint32_t) + sizeof(int64_t) + sizeof(LogType) + sizeof(uint8_t) + sizeof(uint16_t);

// Structure of log record in data file
// crc |tstamp | LogType | keySize | valueSize | key | value
class LogRecord {
  FRIEND_TEST(LogRecordTest, ConstructorTest);

 public:
  LogRecord() = default;

  LogRecord(const LogRecord&) = delete;
  LogRecord& operator=(const LogRecord&) = delete;

  LogRecord(const int32_t& key, const std::string& value, const LogType logType);

  void encode();

  char* getEncodedBuffer() {
    return buf_;
  }

  static std::unique_ptr<LogRecordHeader> decodeLogRecordHeader(char* buf);

  KeyType getKey() {
    return key_;
  }

  std::string getValue() {
    return value_;
  }

  size_t getTotalSize() {
    return totalSize_;
  }

  void setHeader(LogRecordHeader&& header) {
    header_ = std::move(header);
  }

  void loadKVFromBuf(const char* kvBuf, int32_t keySize, int32_t valueSize);

  ~LogRecord() {
    free(buf_);
  }

 private:
  LogRecordHeader header_;
  KeyType key_{0};
  std::string value_;
  size_t totalSize_{0};

  // store encoded str
  char* buf_{nullptr};

  static constexpr int kMaxValueSize = 4096;
};

}  // namespace bitcask

#endif  // DB_LOGRECORD_H_
