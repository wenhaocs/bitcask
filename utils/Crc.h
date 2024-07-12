#ifndef UTILS_CRC_H_
#define UTILS_CRC_H_

#include "bitcask/Base.h"

namespace bitcask {
namespace crc {

uint32_t crc32(const char* data, size_t length);

}  // namespace crc
}  // namespace bitcask

#endif  // UTILS_CRC_H_