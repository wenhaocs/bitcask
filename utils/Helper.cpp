#include "utils/Helper.h"

namespace bitcask {

std::string hexify(const char* buffer, size_t length) {
  std::ostringstream oss;
  for (size_t i = 0; i < length; ++i) {
    oss << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)(unsigned char)buffer[i];
  }
  return oss.str();
}

}