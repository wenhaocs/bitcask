#include "db/DataFile.h"

#include "utils/Crc.h"
#include "utils/Helper.h"

namespace bitcask {

DataFile::DataFile(const std::string dirPath, const uint32_t fileId, bool readOnly) {
  fileName_ = fmt::format("{}/{}", dirPath, fileId);
  curWriteOffset_ = 0;
  readOnly_ = readOnly;
}

Status DataFile::openDataFile() {
  if (readOnly_) {
    FLOG_INFO("Open new data file {} in read only mode.", fileName_);
    fd_ = open(fileName_.c_str(), O_RDONLY, 0444);
  } else {
    FLOG_INFO("Open new data file {} in read write mode.", fileName_);
    fd_ = open(fileName_.c_str(), O_CREAT | O_RDWR | O_APPEND, 0644);
    off_t fileSize = lseek(fd_, 0, SEEK_END);
    if (fileSize == (off_t)-1) {
      return Status::ERROR(Status::Code::kOpenFileError,
                           "Error seeking file: " + std::string(strerror(errno)));
    }
    curWriteOffset_ = fileSize;
  }

  if (fd_ == -1) {
    return Status::ERROR(Status::Code::kOpenFileError,
                         "Error opening file: " + std::string(strerror(errno)));
  } else {
    return Status::OK();
  }
}

Status DataFile::closeDataFile() {
  if (fd_ != -1) {
    close(fd_);
    return Status::OK();
  } else {
    return Status::ERROR(Status::Code::kNoSuchFile,
                         "Error closing file: " + std::string(strerror(errno)));
  }
}

StatusOr<std::unique_ptr<LogRecord>> DataFile::readLogRecord(int64_t pos) {
  // reader header first
  char headerBuf[kLogHeaderSize];
  auto status = readNBytes(pos, kLogHeaderSize, headerBuf);
  if (!status.ok()) {
    LOG(ERROR) << status.message();
    return status;
  }
  auto header = LogRecord::decodeLogRecordHeader(headerBuf);

  FVLOG3("from header, crc: {}, timestamp: {}, log type: {}, key size: {}, value size: {}",
         header->crc_,
         header->tstamp_,
         header->logType_,
         header->keySize_,
         header->valueSize_);

  // Read key and value
  char kvBuf[header->keySize_ + header->valueSize_];
  status = readNBytes(pos + kLogHeaderSize, header->keySize_ + header->valueSize_, kvBuf);
  if (!status.ok()) {
    LOG(ERROR) << status.message();
    return status;
  }

  FVLOG3("kv buf read: {}", hexify(kvBuf, sizeof(kvBuf)));

  auto logRecord = std::make_unique<LogRecord>();
  logRecord->loadKVFromBuf(kvBuf, header->keySize_, header->valueSize_);

  // Reconstruct the encoded data for CRC calculation
  size_t totalSize = kLogHeaderSize - sizeof(header->crc_) + header->keySize_ + header->valueSize_;
  char data[totalSize];
  std::memcpy(data, headerBuf + sizeof(header->crc_), kLogHeaderSize - sizeof(header->crc_));
  std::memcpy(
      data + kLogHeaderSize - sizeof(header->crc_), kvBuf, header->keySize_ + header->valueSize_);

  uint32_t calculatedCRC = crc::crc32(data, totalSize);
  if (calculatedCRC != header->crc_) {
    FLOG_ERROR(
        "CRC validation failed. Crc of read data: {}. Should be {}.", calculatedCRC, header->crc_);
    return Status::ERROR(Status::Code::kError, "CRC validation failed");
  }

  logRecord->setHeader(std::move(*header));

  return logRecord;
}

Status DataFile::readNBytes(int64_t offset, int64_t size, char* buf) {
  auto bytesRead = pread(fd_, buf, size, offset);
  if (bytesRead != size) {
    return Status::ERROR(Status::Code::kError, "Read failure: " + std::string(strerror(errno)));
  }

  FVLOG3("reading {} bytes from {}: {}", size, offset, hexify(buf, size));
  return Status::OK();
}

StatusOr<int64_t> DataFile::writeLogRecord(std::unique_ptr<LogRecord> log) {
  log->encode();

  // Get the encoded buffer
  char* buf = log->getEncodedBuffer();
  size_t totalSize = log->getTotalSize();
  FVLOG3("log to write: {}", hexify(buf, totalSize));

  size_t bytesWritten = 0;
  while (bytesWritten < totalSize) {
    ssize_t result =
        pwrite(fd_, buf + bytesWritten, totalSize - bytesWritten, curWriteOffset_ + bytesWritten);
    if (result == -1) {
      return Status::ERROR(Status::Code::kError, "Write failure" + std::string(strerror(errno)));
    }
    bytesWritten += result;
  }

  int64_t recordPos = curWriteOffset_;
  curWriteOffset_ += totalSize;

  return recordPos;
}

Status DataFile::flush() {
  if (fd_ == -1) {
    return Status::ERROR(Status::Code::kNoSuchFile,
                         "Error flushing file: file descriptor is invalid");
  }

  if (fsync(fd_) == -1) {
    return Status::ERROR(Status::Code::kError,
                         "Error flushing file: " + std::string(strerror(errno)));
  }

  return Status::OK();
}

}  // namespace bitcask