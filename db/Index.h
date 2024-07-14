#ifndef DB_INDEX_H_
#define DB_INDEX_H_

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"
#include "bitcask/Types.h"

namespace bitcask {

struct LogPos {
  FileID fileId_{0};
  uint16_t valueSize_{0};
  FileOffset pos_{0};
  int64_t tstamp_;

  LogPos(const FileID& fileId, const uint16_t& valueSize, const int64_t& pos, const int64_t& tstamp)
      : fileId_(fileId), valueSize_(valueSize), pos_(pos), tstamp_(tstamp) {}
};

class Index {
 public:
  Index() = default;

  virtual Status put(const KeyType& key, std::shared_ptr<LogPos> logPos) = 0;
  virtual StatusOr<std::shared_ptr<LogPos>> get(const KeyType& key) = 0;

  virtual Status remove(const KeyType& key) = 0;

  virtual StatusOr<std::vector<KeyType>> listKeys() = 0;

  struct IterRes {
    KeyType key;
    std::shared_ptr<LogPos> logPos;
  };

  class Iterator {
   public:
    virtual ~Iterator() = default;
    virtual std::unique_ptr<IterRes> next() = 0;
  };

  virtual std::unique_ptr<Iterator> createIterator() = 0;

  Index& operator=(const Index&) = delete;

  virtual ~Index() = default;
};

}  // namespace bitcask

#endif  // DB_INDEX_H_
