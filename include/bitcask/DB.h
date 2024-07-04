#ifndef BITCASK_DB_H_
#define BITCASK_DB_H_

#include "bitcask/Base.h"
#include "bitcask/Options.h"
#include "bitcask/StatusOr.h"

namespace bitcask {

struct Options;
// struct ReadOptions;
// struct WriteOptions;
// class WriteBatch;

// A range of keys
// struct Range {
//   Range() = default;
//   Range(const Slice& s, const Slice& l) : start(s), limit(l) {}

//   Slice start;  // Included in the range
//   Slice limit;  // Not included in the range
// };

// A DB is a persistent ordered map from keys to values.
// A DB is safe for concurrent access from multiple threads without
// any external synchronization.
class DB {
 public:
  // Open a new or existing Bitcask datastore. This is the entry point for DB, and an instance of DB
  // will be returned.
  static StatusOr<std::unique_ptr<DB>> open(const std::string& name, const Options& options);

  // Retrieve a value by key from a Bitcask datastore
  virtual StatusOr<std::string> get(const std::string& key) = 0;

  // Store a key and value in a Bitcask datastore.
  virtual Status put(const std::string& key, const std::string& value) = 0;

  // Delete a key from a Bitcask datastore
  virtual Status deleteKey(const std::string& key) = 0;

  // List all keys in a Bitcask datastore
  virtual StatusOr<std::vector<std::string>> listKeys() = 0;

  // merge the datafiles in the db
  virtual Status merge(const std::string& name) = 0;

  // Force any writes to sync to disk
  virtual Status sync() = 0;

  // Close a Bitcask data store and flush all pending writes (if any) to disk.
  virtual Status close() = 0;

  DB() = default;

  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;

  virtual ~DB();
};

}  // namespace bitcask

#endif  // BITCASK_DB_H_