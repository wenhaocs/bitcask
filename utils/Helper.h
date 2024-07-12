#ifndef UTILS_HELPER_H_
#define UTILS_HELPER_H_

#include "bitcask/Base.h"

namespace bitcask {

std::string hexify(const char* buffer, size_t length);

bool directoryExists(const std::string& path);

bool createDirectory(const std::string& path);

}  // namespace bitcask

#endif  // UTILS_HELPER_H_