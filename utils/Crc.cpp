#include "utils/Crc.h"

namespace bitcask {
namespace crc {

uint32_t crc32(const char* data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;               // Initial CRC value
  const uint32_t polynomial = 0xEDB88320;  // Polynomial used in CRC-32

  for (size_t i = 0; i < length; ++i) {
    uint32_t byte = static_cast<uint8_t>(data[i]);
    crc = crc ^ byte;
    for (uint8_t j = 0; j < 8; ++j) {
      if (crc & 1) {
        crc = (crc >> 1) ^ polynomial;
      } else {
        crc = crc >> 1;
      }
    }
  }
  return ~crc;  // Final XOR value
}

}  // namespace crc
}  // namespace bitcask