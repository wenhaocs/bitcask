#include "db/FileLock.h"

namespace bitcask {

FileLock::FileLock(const std::string& fileName) : fileName_(fileName) {
  fd_ = open(fileName_.c_str(), O_CREAT | O_RDWR, 0666);
}

FileLock::~FileLock() {
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}

Status FileLock::tryLock() {
  if (fd_ == -1)
    return Status::ERROR(Status::Code::kNoSuchFile,
                         "No lock file: " + std::string(strerror(errno)));
  auto ret = flock(fd_, LOCK_EX | LOCK_NB);
  if (ret < 0) {
    FLOG_ERROR("Acquiring db lock error: " + std::string(strerror(errno)));
    return Status::ERROR(Status::Code::kDBUsed,
                         "Acquiring db lock error: " + std::string(strerror(errno)));
  }
  return Status::OK();
}

Status FileLock::unlock() {
  if (fd_ == -1)
    return Status::ERROR(Status::Code::kNoSuchFile,
                         "No lock file: " + std::string(strerror(errno)));
  auto ret = flock(fd_, LOCK_UN);
  return Status::OK();
}
}  // namespace bitcask
