#include "db/DBImpl.h"

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
  LOG(INFO) << "exiit DBImpl";
}

StatusOr<std::unique_ptr<DB>> DB::open(const std::string& dbname, const Options& options) {
  auto db = std::make_unique<DBImpl>(dbname, options);
  return db;
}

// Retrieve a value by key from a Bitcask datastore
StatusOr<std::string> DBImpl::get(const std::string& key) {
  UNUSED(key);
  return "";
}

// Store a key and value in a Bitcask datastore.
Status DBImpl::put(const std::string& key, const std::string& value) {
  UNUSED(key);
  UNUSED(value);
  return Status::OK();
}

// Delete a key from a Bitcask datastore
Status DBImpl::deleteKey(const std::string& key) {
  UNUSED(key);
  return Status::OK();
}

// List all keys in a Bitcask datastore
StatusOr<std::vector<std::string>> DBImpl::listKeys() {
  std::vector<std::string> res;
  return res;
};

// merge the datafiles in the db
Status DBImpl::merge(const std::string& name) {
  UNUSED(name);
  return Status::OK();
}

// Force any writes to sync to disk
Status DBImpl::sync() {
  return Status::OK();
}

// Close a Bitcask data store and flush all pending writes (if any) to disk.
Status DBImpl::close() {
  return Status::OK();
}

DB::~DB() = default;

}  // namespace bitcask
