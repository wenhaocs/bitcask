#include "utils/Helper.h"

namespace bitcask {

std::string hexify(const char* buffer, size_t length) {
  std::ostringstream oss;
  for (size_t i = 0; i < length; ++i) {
    oss << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i];
  }
  return oss.str();
}

bool directoryExists(const std::string& path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    return false;
  } else if (info.st_mode & S_IFDIR) {
    return true;
  } else {
    return false;
  }
}

bool createDirectory(const std::string& path) {
  std::istringstream pathStream(path);
  std::string partialPath = "";
  while (std::getline(pathStream, partialPath, '/')) {
    if (!partialPath.empty()) {
      partialPath = "/" + partialPath;
      if (!directoryExists(partialPath)) {
        if (mkdir(partialPath.c_str(), 0755) != 0 && errno != EEXIST) {
          return false;
        }
      }
    }
  }
  return true;
}

}  // namespace bitcask