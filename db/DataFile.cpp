#include "db/DataFile.h"

#include "utils/Crc.h"
#include "utils/Helper.h"

namespace bitcask {

DataFile::DataFile(const std::string dirPath, const uint32_t fileId, bool readOnly) {
  fileName_ = fmt::format("{}/{}.data", dirPath, fileId);
  curWriteOffset_ = 0;
  readOnly_ = readOnly;
  fileId_ = fileId;
}

Status DataFile::openDataFile() {
  // std::unique_lock<std::shared_mutex> fileLock(fileMutex_);
  if (readOnly_) {
    FLOG_INFO("Open data file {} in read only mode.", fileName_);
    fd_ = open(fileName_.c_str(), O_RDONLY, 0444);
  } else {
    FLOG_INFO("Open data file {} in read write mode.", fileName_);
    fd_ = open(fileName_.c_str(), O_CREAT | O_RDWR | O_APPEND, 0644);
    off_t fileSize = lseek(fd_, 0, SEEK_END);
    if (fileSize == (off_t)-1) {
      FLOG_ERROR("open data file error: {}", std::string(strerror(errno)));
      return Status::ERROR(Status::Code::kOpenFileError,
                           "Error seeking file: " + std::string(strerror(errno)));
    }
    curWriteOffset_ = fileSize;
  }

  if (fd_ == -1) {
    FLOG_ERROR("open data file error: {}", std::string(strerror(errno)));
    return Status::ERROR(Status::Code::kOpenFileError,
                         "Error opening file: " + std::string(strerror(errno)));
  } else {
    return Status::OK();
  }
}

Status DataFile::closeDataFile() {
  // std::unique_lock<std::shared_mutex> fileLock(fileMutex_);
  if (fd_ != -1) {
    close(fd_);
    FVLOG2("[DataFile] Closed data file: {}", fileId_);
    return Status::OK();
  } else {
    return Status::ERROR(Status::Code::kNoSuchFile,
                         "Error closing file: " + std::string(strerror(errno)));
  }
}

StatusOr<std::unique_ptr<LogRecord>> DataFile::readLogRecord(FileOffset pos) {
  // reader header first
  char headerBuf[kLogHeaderSize];
  auto status = readNBytes(pos, kLogHeaderSize, headerBuf);
  if (!status.ok()) {
    return status;
  }
  auto header = LogRecord::decodeLogRecordHeader(headerBuf);

  FVLOG3("from header, crc: {}, timestamp: {}, log type: {}, key size: {}, value size: {}",
         header->crc_,
         header->tstamp_,
         header->logType_,
         header->keySize_,
         header->valueSize_);

  auto valueSize = header->valueSize_;
  auto retrievedCRC = header->crc_;
  auto kvSize = sizeof(KeyType) + valueSize;

  // Read key and value
  char kvBuf[kvSize];
  status = readNBytes(pos + kLogHeaderSize, kvSize, kvBuf);
  if (!status.ok()) {
    return status;
  }

  FVLOG3("kv buf read: {}", hexify(kvBuf, kvSize));

  auto logRecord = std::make_unique<LogRecord>(std::move(header));
  logRecord->loadKVFromBuf(kvBuf, sizeof(KeyType), valueSize);

  // Reconstruct the encoded data for CRC calculation
  size_t sizeWithoutCRC = kLogHeaderSize - sizeof(retrievedCRC) + kvSize;
  char data[sizeWithoutCRC];
  std::memcpy(data, headerBuf + sizeof(retrievedCRC), kLogHeaderSize - sizeof(retrievedCRC));
  std::memcpy(data + kLogHeaderSize - sizeof(retrievedCRC), kvBuf, kvSize);

  uint32_t calculatedCRC = crc::crc32(data, sizeWithoutCRC);
  if (calculatedCRC != retrievedCRC) {
    FLOG_ERROR(
        "CRC validation failed. Crc of read data: {}. Should be {}.", calculatedCRC, retrievedCRC);
    return Status::ERROR(Status::Code::kError, "CRC validation failed");
  }

  return logRecord;
}

StatusOr<std::unique_ptr<LogRecord>> DataFile::readLogRecord(FileOffset pos, uint16_t valueSize) {
  // reader header first
  size_t recordSize = kLogHeaderSize + sizeof(KeyType) + valueSize;
  char logBuf[recordSize];
  auto status = readNBytes(pos, recordSize, logBuf);
  if (!status.ok()) {
    return status;
  }

  FVLOG3("log buf read: {}", hexify(logBuf, sizeof(logBuf)));
  auto header = LogRecord::decodeLogRecordHeader(logBuf);

  FVLOG3("from header, crc: {}, timestamp: {}, log type: {}, key size: {}, value size: {}",
         header->crc_,
         header->tstamp_,
         header->logType_,
         header->keySize_,
         header->valueSize_);

  auto retrievedCRC = header->crc_;
  auto logRecord = std::make_unique<LogRecord>(std::move(header));
  logRecord->loadKVFromBuf(logBuf + kLogHeaderSize, sizeof(KeyType), valueSize);

  // Reconstruct the encoded data for CRC calculation
  size_t sizeWithoutCRC = recordSize - sizeof(retrievedCRC);
  char data[sizeWithoutCRC];
  std::memcpy(data, logBuf + sizeof(retrievedCRC), recordSize - sizeof(retrievedCRC));

  uint32_t calculatedCRC = crc::crc32(data, sizeWithoutCRC);
  if (calculatedCRC != retrievedCRC) {
    FLOG_ERROR(
        "CRC validation failed. Crc of read data: {}. Should be {}.", calculatedCRC, retrievedCRC);
    return Status::ERROR(Status::Code::kError, "CRC validation failed");
  }

  return logRecord;
}

Status DataFile::readNBytes(int64_t offset, int64_t size, char* buf) {
  size_t totalRead = 0;
  while (totalRead < size) {
    auto bytesRead = pread(fd_, buf + totalRead, size - totalRead, offset + totalRead);
    if (bytesRead == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        FLOG_ERROR("Read failure: {}", std::string(strerror(errno)));
        return Status::ERROR(Status::Code::kError, "Read failure: " + std::string(strerror(errno)));
      }
    } else if (bytesRead == 0) {
      return Status::ERROR(Status::Code::kEOF, "EOF");
    }
    totalRead += bytesRead;
  }

  FVLOG3("reading {} bytes from {}: {}", size, offset, hexify(buf, size));
  return Status::OK();
}

StatusOr<FileOffset> DataFile::writeLogRecord(std::unique_ptr<LogRecord>&& log) {
  FVLOG2("[DataFile] Writing to data file: {}", fileId_);

  log->encode();

  // Get the encoded buffer
  char* buf = log->getEncodedBuffer();
  size_t totalSize = log->getTotalSize();
  FVLOG3("log to write: {}", hexify(buf, totalSize));
  FileOffset recordPos = 0;
  // std::unique_lock<std::shared_mutex> fileLock(fileMutex_);
  size_t bytesWritten = 0;
  while (bytesWritten < totalSize) {
    ssize_t result =
        pwrite(fd_, buf + bytesWritten, totalSize - bytesWritten, curWriteOffset_ + bytesWritten);
    if (result == -1) {
      FLOG_ERROR("Write failure: {}", std::string(strerror(errno)));
      return Status::ERROR(Status::Code::kError, "Write failure" + std::string(strerror(errno)));
    }
    bytesWritten += result;
  }

  recordPos = curWriteOffset_;
  curWriteOffset_ += totalSize;
  return recordPos;
}

Status DataFile::flush() {
  // std::unique_lock<std::shared_mutex> fileLock(fileMutex_);
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

int64_t DataFile::getCurrentFileSize() {
  // std::shared_lock<std::shared_mutex> fileLock(fileMutex_);
  return curWriteOffset_;
}

}  // namespace bitcask