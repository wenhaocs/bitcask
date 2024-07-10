#include "db/DBImpl.h"

#include "bitcask/Types.h"
#include "db/HashIndex.h"
#include "utils/Helper.h"

namespace bitcask {

DBImpl::DBImpl(const std::string& dbname, const Options& options)
    : options_(options), dbname_(dbname) {}

DBImpl::~DBImpl() {
  // Wait for background work to finish.
  //   mutex_.Lock();
  //   shutting_down_.store(true, std::memory_order_release);
  //   while (background_compaction_scheduled_) {
  //     background_work_finished_signal_.Wait();
  //   }
  //   mutex_.Unlock();

  //   if (db_lock_ != nullptr) {
  //     env_->UnlockFile(db_lock_);
  //   }

  //   delete versions_;
  //   if (mem_ != nullptr) mem_->Unref();
  //   if (imm_ != nullptr) imm_->Unref();
  //   delete tmp_batch_;
  //   delete log_;
  //   delete logfile_;
  //   delete table_cache_;

  //   if (owns_info_log_) {
  //     delete options_.info_log;
  //   }
  //   if (owns_cache_) {
  //     delete options_.block_cache;
  //   }
  LOG(INFO) << "exit DBImpl";
}

StatusOr<std::unique_ptr<DB>> DB::open(const std::string& dbname, const Options& options) {
  // check options

  if (!directoryExists(dbname)) {
    if (!createDirectory(dbname)) {
      FLOG_ERROR("Failed to create db path: {}", std::string(strerror(errno)));
      return Status::ERROR(Status::Code::kError, std::string(strerror(errno)));
    }
  }

  auto dbImpl = std::make_unique<DBImpl>(dbname, options);

  // Try to lock the lock file. Is it's already acquired by another process, refuse to open.
  if (!options.readOnly) {
    dbImpl->fileLock_ = std::make_unique<FileLock>(dbImpl->fileLockName_);
    auto status = dbImpl->fileLock_->tryLock();
    if (!status.ok()) {
      return status;
    }
  }

  // TODO: Replay WAL

  // load active data file
  auto status = dbImpl->openActiveDataFile();
  if (!status.ok()) {
    return status;
  }

  // Load index from data files

  return dbImpl;
}

// Retrieve a value by key from a Bitcask datastore
StatusOr<std::string> DBImpl::get(const std::string& key) {
  UNUSED(key);
  // search the index
  // read from disk
  return "";
}

// Store a key and value in a Bitcask datastore.
// Note that the on disk part is written first then the in memory index. There is no need of
// additional WAL.
Status DBImpl::put(const std::string& key, const std::string& value) {
  UNUSED(key);
  UNUSED(value);

  // Write to WAL

  // Construct log record

  // Append log record

  // Update index

  return Status::OK();
}

// Delete a key from a Bitcask datastore
Status DBImpl::deleteKey(const std::string& key) {
  UNUSED(key);
  // check if key exists

  // Write to WAL

  // Construct log record

  // Append log record

  // delete in index

  return Status::OK();
}

// List all keys in a Bitcask datastore
StatusOr<std::vector<std::string>> DBImpl::listKeys() {
  std::vector<std::string> res;

  // iterate the index
  return res;
};

// merge the datafiles in the db
Status DBImpl::merge(const std::string& name) {
  UNUSED(name);
  return Status::OK();
}

// Force any writes to sync to disk
Status DBImpl::sync() {
  // Sync active data file
  return Status::OK();
}

// Close a Bitcask data store and flush all pending writes (if any) to disk.
Status DBImpl::close() {
  // sync
  // unlock the lock file
  return Status::OK();
}

DB::~DB() = default;

Status DBImpl::openActiveDataFile() {
  // Iterate through the directory
  for (const auto& entry : std::filesystem::directory_iterator(dbname_)) {
    if (entry.is_regular_file()) {
      auto path = entry.path();
      if (path.extension() == ".data") {
        // Extract file ID from filename
        std::string filename = path.stem().string();
        try {
          FileID fileId = std::stoul(filename);
          allFileIDs_.emplace_back(std::move(fileId));
        } catch (const std::invalid_argument& e) {
          // Handle invalid filenames that can't be converted to a number
          FLOG_ERROR("Invalid filename: {}", filename);
          return Status::ERROR(Status::Code::kError, "Invalid filename: " + filename);
        }
      }
    }
  }

  // Find the largest file ID
  FileID activeFileId;
  if (!allFileIDs_.empty()) {
    auto activeFileId = *std::max_element(allFileIDs_.begin(), allFileIDs_.end());
  } else {
    // It's a new database.
    if (options_.readOnly) {
      FLOG_ERROR("Open in read only mode but there are no data files.");
      return Status::ERROR(Status::Code::kError, "Empty data file.");
    } else {
      // Create the first data file.
      activeFileId = 1;
    }
  }

  auto activeFilePath = dbname_ + "/" + std::to_string(activeFileId) + ".data";
  auto activeFile_ = std::make_unique<DataFile>(activeFilePath, activeFileId, options_.readOnly);
  auto status = activeFile_->openDataFile();
  if (!status.ok()) {
    return status;
  }

  return Status::OK();
}

Status DBImpl::constructIndex() {
  index_ = std::make_unique<HashIndex>(100);

  for (const auto& fileId : allFileIDs_) {
    auto path = dbname_ + "/" + std::to_string(fileId) + ".data";
    auto dataFile = std::make_unique<DataFile>(path, fileId, true);
    auto status = dataFile->openDataFile();
    if (!status.ok()) {
      return status;
    }

    int64_t pos = 0;
    while (true) {
      auto result = dataFile->readLogRecord(pos);
      if (!result.ok()) {
        if (result.status().code() == Status::Code::kEOF) {
          break;  // End of file reached
        }
        return result.status();
      }

      auto logRecord = std::move(result.value());
      auto key = logRecord->getKey();
      auto logPos = std::make_shared<LogPos>(
          fileId, logRecord->getValueSize(), pos, logRecord->getTimeStamp());
      index_->put(key, std::move(logPos));

      pos += logRecord->getTotalSize();
    }

    dataFile->closeDataFile();
  }

  return Status::OK();
}
}  // namespace bitcask
