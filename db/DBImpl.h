#ifndef DB_DB_IMPL_H_
#define DB_DB_IMPL_H_

#include "bitcask/Base.h"
#include "bitcask/DB.h"

namespace bitcask {

class DBImpl : public DB {
 public:
  DBImpl(const std::string& dbname, const Options& options);

  DBImpl(const DBImpl&) = delete;
  DBImpl& operator=(const DBImpl&) = delete;

  ~DBImpl() override;

  // Retrieve a value by key from a Bitcask datastore
  StatusOr<std::string> get(const std::string& key) override;

  // Store a key and value in a Bitcask datastore.
  Status put(const std::string& key, const std::string& value) override;

  // Delete a key from a Bitcask datastore
  Status deleteKey(const std::string& key) override;

  // List all keys in a Bitcask datastore
  StatusOr<std::vector<std::string>> listKeys() override;

  // merge the datafiles in the db
  Status merge(const std::string& name) override;

  // Force any writes to sync to disk
  Status sync() override;

  // Close a Bitcask data store and flush all pending writes (if any) to disk.
  Status close() override;

 private:
  friend class DB;

  const Options options_;
  const std::string dbname_;
};

}  // namespace bitcask

#endif  // DB_DB_IMPL_H_
