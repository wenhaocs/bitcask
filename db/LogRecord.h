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

  LogRecordHeader() = default;
  LogRecordHeader(const int64_t& tstamp,
                  const LogType& logType,
                  const uint8_t& keySize,
                  const uint16_t& valueSize)
      : tstamp_(tstamp), logType_(logType), keySize_(keySize), valueSize_(valueSize) {}
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

  LogRecord(const KeyType& key, const std::string& value, const LogType logType);

  explicit LogRecord(std::unique_ptr<LogRecordHeader> header);

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

  uint16_t getValueSize() {
    return header_->valueSize_;
  }

  int64_t getTimeStamp() {
    return header_->tstamp_;
  }

  LogType getLogType() {
    return header_->logType_;
  }

  void loadKVFromBuf(const char* kvBuf, int32_t keySize, int32_t valueSize);

  ~LogRecord() {
    // The buf_ is freed with destruction of the LogRecord, usually after put.
    free(buf_);
  }

 private:
  std::unique_ptr<LogRecordHeader> header_{nullptr};
  KeyType key_{0};
  std::string value_;
  size_t totalSize_{0};

  // store encoded str
  char* buf_{nullptr};

  static constexpr int kMaxValueSize = 4096;
};

}  // namespace bitcask

#endif  // DB_LOGRECORD_H_
