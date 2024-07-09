#ifndef DB_INDEX_H_
#define DB_INDEX_H_

#include "bitcask/Base.h"
#include "bitcask/StatusOr.h"
#include "bitcask/Types.h"

namespace bitcask {

struct LogPos {
  uint32_t fileId_{0};
  uint16_t valueSize_{0};
  int64_t pos_{0};
  int64_t tstamp_;

  LogPos(uint32_t fileId, uint16_t valueSize, int64_t pos, int64_t tstamp)
      : fileId_(fileId), valueSize_(valueSize), pos_(pos), tstamp_(tstamp) {}
};

class Index {
 public:
  Index() = default;

  virtual Status put(const KeyType& key, std::shared_ptr<LogPos> logPos) = 0;
  virtual StatusOr<std::shared_ptr<LogPos>> get(const KeyType& key) = 0;

  virtual Status remove(const KeyType& key) = 0;

  Index& operator=(const Index&) = delete;

  virtual ~Index() = default;
};

}  // namespace bitcask

#endif  // DB_INDEX_H_
