#include "db/LogRecord.h"

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

  // Calculate CRC-32 of the encoded buffer
  std::vector<uint8_t> crcData(buf_.begin(), buf_.end());
  crc_ = crc::crc32(crcData);

  // Encode the key, value, timestamp, logType into buf_ for CRC calculation
  buf_.clear();
  buf_.append(reinterpret_cast<const char*>(&crc_), sizeof(crc_));
  buf_.append(reinterpret_cast<const char*>(&tstamp_), sizeof(tstamp_));
  buf_.append(reinterpret_cast<const char*>(&logType_), sizeof(logType_));
  buf_.append(reinterpret_cast<const char*>(&keySize_), sizeof(keySize_));
  buf_.append(reinterpret_cast<const char*>(&valueSize_), sizeof(valueSize_));
  buf_.append(reinterpret_cast<const char*>(&key_), sizeof(key_));
  buf_.append(value_.data(), value_.size());
}

}  // namespace bitcask
