#include "db/LogRecord.h"

#include "utils/Crc.h"
#include "utils/WallClock.h"

namespace bitcask {

LogRecord::LogRecord(const int32_t& key, const std::string& value, const LogType logType) {
  // Ensure the value size does not exceed the maximum allowed size
  if (value.size() > kMaxValueSize) {
    throw std::length_error("Value size exceeds maximum allowed size");
  }

  header_.keySize_ = 4;
  header_.valueSize_ = static_cast<uint16_t>(value.size());
  header_.tstamp_ = time::WallClock::fastNowInMicroSec();
  header_.logType_ = logType;

  key_ = key;
  value_ = value;
  totalSize_ = kLogHeaderSize + sizeof(key_) + value_.size();
}

LogRecord::LogRecord(const LogRecordHeader& header) : header_(header) {}

void LogRecord::encode() {
  // total size is awalys set together with buf_
  buf_ = reinterpret_cast<char*>(malloc(totalSize_));
  // Encode the key, value, timestamp, logType into buf_ for CRC calculation
  int index = 0;
  std::memcpy(buf_ + index, reinterpret_cast<const char*>(&header_.crc_), sizeof(header_.crc_));
  index += sizeof(header_.crc_);
  std::memcpy(
      buf_ + index, reinterpret_cast<const char*>(&header_.tstamp_), sizeof(header_.tstamp_));
  index += sizeof(header_.tstamp_);
  std::memcpy(
      buf_ + index, reinterpret_cast<const char*>(&header_.logType_), sizeof(header_.logType_));
  index += sizeof(header_.logType_);
  std::memcpy(
      buf_ + index, reinterpret_cast<const char*>(&header_.keySize_), sizeof(header_.keySize_));
  index += sizeof(header_.keySize_);
  std::memcpy(
      buf_ + index, reinterpret_cast<const char*>(&header_.valueSize_), sizeof(header_.valueSize_));
  index += sizeof(header_.valueSize_);

  std::memcpy(buf_ + index, reinterpret_cast<const char*>(&key_), sizeof(key_));
  index += sizeof(key_);
  std::memcpy(buf_ + index, value_.data(), value_.size());

  // Calculate CRC-32 of the encoded buffer
  auto crcSize = sizeof(header_.crc_);
  uint32_t crcValue = crc::crc32(buf_ + crcSize, totalSize_ - crcSize);
  memcpy(buf_, reinterpret_cast<const char*>(&crcValue), sizeof(crcValue));
}

std::unique_ptr<LogRecordHeader> LogRecord::decodeLogRecordHeader(char* buf) {
  auto header = std::make_unique<LogRecordHeader>();

  int index = 0;
  std::memcpy(&header->crc_, buf + index, sizeof(header->crc_));
  index += sizeof(header->crc_);

  std::memcpy(&header->tstamp_, buf + index, sizeof(header->tstamp_));
  index += sizeof(header->tstamp_);

  std::memcpy(&header->logType_, buf + index, sizeof(header->logType_));
  index += sizeof(header->logType_);

  std::memcpy(&header->keySize_, buf + index, sizeof(header->keySize_));
  index += sizeof(header->keySize_);

  std::memcpy(&header->valueSize_, buf + index, sizeof(header->valueSize_));
  index += sizeof(header->valueSize_);

  return header;
}

void LogRecord::loadKVFromBuf(const char* kvBuf, int32_t keySize, int32_t valueSize) {
  std::memcpy(&key_, kvBuf, keySize);
  value_.assign(kvBuf + keySize, valueSize);
  totalSize_ = kLogHeaderSize + keySize + valueSize;
  FVLOG2("Set kv from buf. key: {}, value: {}", key_, value_);
}

}  // namespace bitcask
