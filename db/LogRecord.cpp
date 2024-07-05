#include "db/LogRecord.h"

#include "utils/Crc.h"
#include "utils/WallClock.h"

namespace bitcask {

LogRecord::LogRecord(const int32_t& key, const std::string& value, const LogType logType)
    : key_(key),
      value_(value),
      logType_(logType),
      valueSize_(static_cast<uint16_t>(value.size())),
      tstamp_(time::WallClock::fastNowInMicroSec()) {
  // Ensure the value size does not exceed the maximum allowed size
  if (value.size() > kMaxValueSize) {
    throw std::length_error("Value size exceeds maximum allowed size");
  }

  buf_ = reinterpret_cast<char*>(malloc(sizeof(uint32_t) + sizeof(int64_t) + sizeof(LogType) +
                                        sizeof(uint8_t) + sizeof(uint16_t) + sizeof(int32_t) +
                                        value_.size()));
}

void LogRecord::encode() {
  // Encode the key, value, timestamp, logType into buf_ for CRC calculation
  int index = 0;
  index += sizeof(uint32_t);
  memcpy(buf_ + index, reinterpret_cast<const char*>(&tstamp_), sizeof(tstamp_));
  index += sizeof(int64_t);
  memcpy(buf_ + index, reinterpret_cast<const char*>(&logType_), sizeof(logType_));
  index += sizeof(LogType);
  memcpy(buf_ + index, reinterpret_cast<const char*>(&keySize_), sizeof(keySize_));
  index += sizeof(uint8_t);
  memcpy(buf_ + index, reinterpret_cast<const char*>(&valueSize_), sizeof(valueSize_));
  index += sizeof(uint16_t);
  memcpy(buf_ + index, reinterpret_cast<const char*>(&key_), sizeof(key_));
  index += sizeof(int32_t);
  memcpy(buf_ + index, value_.data(), value_.size());

  // Calculate CRC-32 of the encoded buffer
  crc_ = crc::crc32(buf_, sizeof(buf_));
  memcpy(buf_, reinterpret_cast<const char*>(&crc_), sizeof(crc_));
}

}  // namespace bitcask
