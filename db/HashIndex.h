#ifndef DB_HASHINDEX_H_
#define DB_HASHINDEX_H_

#include "db/Index.h"

namespace bitcask {

class HashIndex : Index {
 public:
  HashIndex() = default;
  ~HashIndex() = default;

  explicit HashIndex(size_t initialSize);

  Status put(const KeyType& key, std::shared_ptr<LogPos> logPos) override;

  StatusOr<std::shared_ptr<LogPos>> get(const KeyType& key) override;

  Status remove(const KeyType& key) override;

 private:
  // since it's in memory structure, we can keep using the ponter and share the ownership to reduce
  // memory footprint
  std::unordered_map<KeyType, std::shared_ptr<LogPos>> indexMap_;
  mutable std::shared_mutex mutex_;
};

}  // namespace bitcask

#endif  // DB_HASHINDEX_H_
