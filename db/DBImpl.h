#ifndef DB_DB_IMPL_H_
#define DB_DB_IMPL_H_

#include <gtest/gtest_prod.h>

#include "bitcask/Base.h"
#include "bitcask/DB.h"
#include "bitcask/Types.h"
#include "db/DataFile.h"
#include "db/FileLock.h"
#include "db/Index.h"

DECLARE_uint64(max_value_size);

namespace bitcask {

class DBImpl : public DB {
  FRIEND_TEST(DBImplTest, PutExceedingFileLimitTest);

 public:
  DBImpl(const std::string& dbname, const Options& options);

  DBImpl(const DBImpl&) = delete;
  DBImpl& operator=(const DBImpl&) = delete;

  ~DBImpl() override;

  // Retrieve a value by key from a Bitcask datastore
  StatusOr<std::string> get(const KeyType& key) override;

  // Store a key and value in a Bitcask datastore.
  Status put(const KeyType& key, const std::string& value) override;

  // Delete a key from a Bitcask datastore
  Status deleteKey(const KeyType& key) override;

  // List all keys in a Bitcask datastore
  StatusOr<std::vector<KeyType>> listKeys() override;

  // merge the datafiles in the db
  Status merge(const std::string& name) override;

  // Force any writes to sync to disk
  Status sync() override;

  // Close a Bitcask data store and flush all pending writes (if any) to disk.
  Status close() override;

 private:
  // load all data files
  // This function should only be called in open. It does not require additional lock as it's
  // protected by the file lock and there can't be race condition on this.
  Status openAllDataFiles();

  // construct in memory index from all data files
  // This function should only be called in open. It does not require additional lock as it's
  // protected by the file lock and there can't be race condition on this.
  Status constructIndex();

  // Internally manage active datafile and append logRecord.
  // It needs to read the current offset inside active file to determine whether the incoming write
  // will exceed the max file limit. If so, create a new active file. This function is called inside
  // put, so there can be race condition. Need to synchronize on the operations on activeFile_.
  StatusOr<FileOffset> appendLogRecord(std::unique_ptr<LogRecord>&& logRecod);

  bool checkValue(const std::string& value);

  std::unique_ptr<FileLock> fileLock_{nullptr};
  FileID activeFileId_{0};
  std::vector<FileID> allFileIds_;
  std::unique_ptr<DataFile> activeFile_{nullptr};
  std::unordered_map<FileID, std::unique_ptr<DataFile>> oldDataFiles_;
  std::unique_ptr<Index> index_{nullptr};

  mutable std::shared_mutex mutex_;

  friend class DB;

  const Options options_;
  const std::string dbname_;

  const std::string fileLockName_ = "LOCK";
};

}  // namespace bitcask

#endif  // DB_DB_IMPL_H_
