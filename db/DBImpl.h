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

  // Implementations of the DB interface
  //   Status Put(const WriteOptions&, const Slice& key, const Slice& value) override;
  //   Status Delete(const WriteOptions&, const Slice& key) override;
  //   Status Write(const WriteOptions& options, WriteBatch* updates) override;
  //   Status Get(const ReadOptions& options, const Slice& key, std::string* value) override;
  //   Iterator* NewIterator(const ReadOptions&) override;
  //   const Snapshot* GetSnapshot() override;
  //   void ReleaseSnapshot(const Snapshot* snapshot) override;
  //   bool GetProperty(const Slice& property, std::string* value) override;
  //   void GetApproximateSizes(const Range* range, int n, uint64_t* sizes) override;
  //   void CompactRange(const Slice* begin, const Slice* end) override;

  // Extra methods (for testing) that are not in the public DB interface

  // Compact any files in the named level that overlap [*begin,*end]
  //   void TEST_CompactRange(int level, const Slice* begin, const Slice* end);

  // Force current memtable contents to be compacted.
  //   Status TEST_CompactMemTable();

  // Return an internal iterator over the current state of the database.
  // The keys of this iterator are internal keys (see format.h).
  // The returned iterator should be deleted when no longer needed.
  //   Iterator* TEST_NewInternalIterator();

  // Return the maximum overlapping data (in bytes) at next level for any
  // file at a level >= 1.
  //   int64_t TEST_MaxNextLevelOverlappingBytes();

  // Record a sample of bytes read at the specified internal key.
  // Samples are taken approximately once every config::kReadBytesPeriod
  // bytes.
  //   void RecordReadSample(Slice key);

 private:
  friend class DB;

  const Options options_;
  const std::string dbname_;
};

}  // namespace bitcask

#endif  // DB_DB_IMPL_H_
