#ifndef UTILS_CRC_H_
#define UTILS_CRC_H_

#include "bitcask/Base.h"

namespace bitcask {
namespace crc {

inline uint32_t crc32(const std::vector<uint8_t>& data);

}  // namespace crc
}  // namespace bitcask

#endif  // UTILS_CRC_H_