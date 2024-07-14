#ifndef DB_HASHINDEX_H_
#define DB_HASHINDEX_H_

#include "db/Index.h"

namespace bitcask {

class HashIndex : public Index {
 public:
  HashIndex() = default;
  ~HashIndex() = default;

  explicit HashIndex(size_t initialSize);

  Status put(const KeyType& key, std::shared_ptr<LogPos> logPos) override;

  StatusOr<std::shared_ptr<LogPos>> get(const KeyType& key) override;

  Status remove(const KeyType& key) override;

  StatusOr<std::vector<KeyType>> listKeys() override;

  class HashIndexIterator : public Iterator {
   public:
    // We are using std::map iterator, need to synchronize with writes.
    // Due to the limitation of unordered_map, no writes are allowed during iteration, vice versa.
    explicit HashIndexIterator(const HashIndex& hashIndex)
        : mutex_(hashIndex.mutex_),
          it_(hashIndex.indexMap_.begin()),
          end_(hashIndex.indexMap_.end()) {
      mutex_.lock_shared();
    }

    ~HashIndexIterator() override {
      mutex_.unlock_shared();
    }

    std::unique_ptr<IterRes> next() override {
      if (it_ == end_) {
        return nullptr;
      }
      auto res = std::make_unique<IterRes>();
      res->key = it_->first;
      res->logPos = it_->second;
      ++it_;
      return res;
    }

   private:
    std::shared_mutex& mutex_;
    std::unordered_map<KeyType, std::shared_ptr<LogPos>>::const_iterator it_;
    std::unordered_map<KeyType, std::shared_ptr<LogPos>>::const_iterator end_;
  };

  std::unique_ptr<Iterator> createIterator() override;

  HashIndex& operator=(const HashIndex&) = delete;

 private:
  // since it's in memory structure, we can keep using the ponter and share the ownership to reduce
  // memory footprint
  // TODO: Replace with folly::ConcurrentHashMap
  std::unordered_map<KeyType, std::shared_ptr<LogPos>> indexMap_;
  mutable std::shared_mutex mutex_;
};

}  // namespace bitcask

#endif  // DB_HASHINDEX_H_
