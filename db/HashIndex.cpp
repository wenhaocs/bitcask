#include "db/HashIndex.h"

namespace bitcask {

HashIndex::HashIndex(size_t initialSize) {
  indexMap_.reserve(initialSize);
}

Status HashIndex::put(const KeyType& key, std::shared_ptr<LogPos> logPos) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  indexMap_[key] = std::move(logPos);
  return Status::OK();
}

StatusOr<std::shared_ptr<LogPos>> HashIndex::get(const KeyType& key) {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto it = indexMap_.find(key);
  if (it != indexMap_.end()) {
    return it->second;
  } else {
    return Status::ERROR(Status::Code::kNotFound, "Key not found");
  }
}

Status HashIndex::remove(const KeyType& key) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  auto it = indexMap_.find(key);
  if (it != indexMap_.end()) {
    indexMap_.erase(it);
    return Status::OK();
  } else {
    return Status::ERROR(Status::Code::kNotFound, "Key not found");
  }
}

}  // namespace bitcask