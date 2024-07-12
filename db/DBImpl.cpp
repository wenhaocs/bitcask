#include "db/DBImpl.h"

#include "db/HashIndex.h"
#include "utils/Helper.h"

DEFINE_uint64(max_value_size,
              4096,
              "Max length for the value string. The max of this value is 65535");

namespace bitcask {

DBImpl::DBImpl(const std::string& dbname, const Options& options)
    : options_(options), dbname_(dbname) {}

DBImpl::~DBImpl() {
  close();
  LOG(INFO) << "exit DBImpl";
}

StatusOr<std::unique_ptr<DB>> DB::open(const std::string& dbname, const Options& options) {
  // check options
  if (options.readOnly) {
    FLOG_INFO("Trying to open db in read only mode...");
  } else {
    FLOG_INFO("Trying to open db in rw mode...");
  }

  if (!directoryExists(dbname)) {
    if (!createDirectory(dbname)) {
      FLOG_ERROR("Failed to create db path {}: {}", dbname, std::string(strerror(errno)));
      return Status::ERROR(Status::Code::kError, std::string(strerror(errno)));
    }
  }

  auto dbImpl = std::make_unique<DBImpl>(dbname, options);

  // Try to lock the lock file. Is it's already acquired by another process, refuse to open.
  if (!options.readOnly) {
    FLOG_INFO("Locking db...");
    dbImpl->fileLock_ = std::make_unique<FileLock>(dbImpl->fileLockName_);
    auto status = dbImpl->fileLock_->tryLock();
    if (!status.ok()) {
      return status;
    }
    FLOG_INFO("DB locked successfully");
  }

  // TODO: Replay WAL

  // load active data file
  FLOG_INFO("Loading active data file...");
  auto status = dbImpl->openAllDataFiles();
  if (!status.ok()) {
    return status;
  }

  // Load index from data files
  FLOG_INFO("Constructing index...");
  status = dbImpl->constructIndex();
  if (!status.ok()) {
    return status;
  }

  return dbImpl;
}

// Retrieve a value by key from a Bitcask datastore
StatusOr<std::string> DBImpl::get(const KeyType& key) {
  // search the index
  auto ret = index_->get(key);
  if (!ret.ok()) {
    return ret.status();
  }

  // read from disk
  auto logPos = std::move(ret).value();
  StatusOr<std::unique_ptr<LogRecord>> logRet;
  if (logPos->fileId_ == activeFileId_) {
    logRet = activeFile_->readLogRecord(logPos->pos_, logPos->valueSize_);
  } else {
    if (oldDataFiles_.find(logPos->fileId_) == oldDataFiles_.end()) {
      FLOG_ERROR("Data file not found: {}", logPos->fileId_);
      return Status::ERROR(Status::Code::kNoSuchFile, "data file not found.");
    }
    logRet = oldDataFiles_.at(logPos->fileId_)->readLogRecord(logPos->pos_, logPos->valueSize_);
  }

  if (!logRet.ok()) {
    return logRet.status();
  }

  return std::move(logRet).value()->getValue();
}

// Store a key and value in a Bitcask datastore.
// Note that the on disk part is written first then the in memory index. There is no need of
// additional WAL.
Status DBImpl::put(const KeyType& key, const std::string& value) {
  if (UNLIKELY(options_.readOnly)) {
    return Status::ERROR(Status::Code::kNotAllowed, "write is not allowd in read only mode");
  }
  if (!checkValue(value)) {
    FLOG_ERROR("Value size over limit. Please check FLAGS_max_value_size");
    return Status::ERROR(Status::Code::kOverLimit, "Value size over limit.");
  }

  // TODO: Write to WAL

  // Write to file first. In case of failure, we can reconstruct index from file.
  auto logRecord = std::make_unique<LogRecord>(key, value, LogType::WRITE);
  auto valueSize = logRecord->getValueSize();
  auto tstamp = logRecord->getTimeStamp();
  auto ret = appendLogRecord(std::move(logRecord));
  if (!ret.ok()) {
    return ret.status();
  }

  auto offset = std::move(ret).value();

  // Update index
  auto logPos = std::make_shared<LogPos>(
      activeFileId_, std::move(valueSize), std::move(offset), std::move(tstamp));
  index_->put(key, logPos);

  return Status::OK();
}

// Delete a key from a Bitcask datastore
Status DBImpl::deleteKey(const KeyType& key) {
  // check if key exists
  auto ret = index_->get(key);
  if (!ret.ok()) {
    return ret.status();
  }

  // TODO: Write to WAL

  // Construct log record
  auto logRecord = std::make_unique<LogRecord>(key, "", LogType::DELETE);
  auto appendRet = appendLogRecord(std::move(logRecord));
  if (!appendRet.ok()) {
    return appendRet.status();
  }

  // delete in index
  index_->remove(key);

  return Status::OK();
}

// List all keys in a Bitcask datastore
StatusOr<std::vector<KeyType>> DBImpl::listKeys() {
  auto ret = index_->listKeys();
  if (!ret.ok()) {
    return ret.status();
  }
  return std::move(ret).value();
};

// merge the datafiles in the db
Status DBImpl::merge(const std::string& name) {
  UNUSED(name);
  return Status::OK();
}

// Force any writes to sync to disk
Status DBImpl::sync() {
  // Sync active data file
  return activeFile_->flush();
}

// Close a Bitcask data store and flush all pending writes (if any) to disk.
Status DBImpl::close() {
  sync();
  fileLock_->unlock();
  return Status::OK();
}

DB::~DB() = default;

Status DBImpl::openAllDataFiles() {
  // Iterate through the directory
  for (const auto& entry : std::filesystem::directory_iterator(dbname_)) {
    if (entry.is_regular_file()) {
      auto path = entry.path();
      if (path.extension() == ".data") {
        // Extract file ID from filename
        std::string filename = path.stem().string();
        try {
          FileID fileId = std::stoul(filename);
          allFileIds_.emplace_back(std::move(fileId));
        } catch (const std::invalid_argument& e) {
          // Handle invalid filenames that can't be converted to a number
          FLOG_ERROR("Invalid filename: {}", filename);
          return Status::ERROR(Status::Code::kError, "Invalid filename: " + filename);
        }
      }
    }
  }

  if (!allFileIds_.empty()) {
    // Find the largest file ID
    std::sort(allFileIds_.begin(), allFileIds_.end());
    activeFileId_ = allFileIds_.back();

    FLOG_INFO("Active file id: {}", activeFileId_);

    for (const auto& fileId : allFileIds_) {
      if (fileId != activeFileId_) {
        auto oldFile = std::make_unique<DataFile>(dbname_, fileId, true);
        auto status = oldFile->openDataFile();
        if (!status.ok()) {
          return status;
        }
        oldDataFiles_.emplace(fileId, std::move(oldFile));
      } else {
        activeFile_ = std::make_unique<DataFile>(dbname_, activeFileId_, options_.readOnly);
        auto status = activeFile_->openDataFile();
        if (!status.ok()) {
          return status;
        }
      }
    }
  } else {
    FLOG_INFO("New database!");
    // It's a new database.
    if (options_.readOnly) {
      FLOG_ERROR("Open in read only mode but there are no data files.");
      return Status::ERROR(Status::Code::kError, "Empty data file.");
    } else {
      // Create the first data file.
      activeFileId_ = 1;
      allFileIds_.emplace_back(activeFileId_);

      activeFile_ = std::make_unique<DataFile>(dbname_, activeFileId_);
      auto status = activeFile_->openDataFile();
      if (!status.ok()) {
        return status;
      }
    }
  }

  return Status::OK();
}

Status DBImpl::constructIndex() {
  index_ = std::make_unique<HashIndex>(100);

  for (const auto& fileId : allFileIds_) {
    DataFile* curDatafile{nullptr};
    if (fileId == activeFileId_) {
      curDatafile = activeFile_.get();
    } else {
      if (oldDataFiles_.find(fileId) == oldDataFiles_.end()) {
        FLOG_ERROR("Data file not found: {}", fileId);
        return Status::ERROR(Status::Code::kNoSuchFile, "data file not found.");
      }
      curDatafile = oldDataFiles_.at(fileId).get();
    }

    int64_t pos = 0;
    while (true) {
      FVLOG2("Loading index from data file {}", fileId);
      auto result = curDatafile->readLogRecord(pos);
      if (!result.ok()) {
        if (result.status().code() == Status::Code::kEOF) {
          break;  // End of file reached
        }
        return result.status();
      }

      auto logRecord = std::move(result.value());
      auto key = logRecord->getKey();

      if (logRecord->getLogType() == LogType::WRITE) {
        auto logPos = std::make_shared<LogPos>(
            fileId, logRecord->getValueSize(), pos, logRecord->getTimeStamp());
        index_->put(key, std::move(logPos));
      } else {
        index_->remove(key);
      }

      pos += logRecord->getTotalSize();
    }
  }

  return Status::OK();
}

StatusOr<FileOffset> DBImpl::appendLogRecord(std::unique_ptr<LogRecord>&& logRecord) {
  // rolling out data file and write must be atomic
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (activeFile_->getCurrentFileSize() + logRecord->getTotalSize() > options_.maxFileSize) {
    // roll out a new data file
    activeFile_->flush();
    activeFile_->closeDataFile();

    // reopen this data file as read only mode and append to old datafiles
    auto oldFile = std::make_unique<DataFile>(dbname_, activeFileId_, true);
    auto status = oldFile->openDataFile();
    if (!status.ok()) {
      return status;
    }
    oldDataFiles_.emplace(activeFileId_, std::move(oldFile));

    // create new active data file
    activeFileId_++;
    allFileIds_.emplace_back(activeFileId_);
    activeFile_ = std::make_unique<DataFile>(dbname_, activeFileId_);
    status = activeFile_->openDataFile();
    if (!status.ok()) {
      return status;
    }
    FLOG_INFO("Rolled out a new data file: {}", activeFileId_);
  }
  auto ret = activeFile_->writeLogRecord(std::move(logRecord));
  if (!ret.ok()) {
    return ret.status();
  }
  return std::move(ret).value();
}

bool DBImpl::checkValue(const std::string& value) {
  return value.size() <= FLAGS_max_value_size;
}
}  // namespace bitcask
