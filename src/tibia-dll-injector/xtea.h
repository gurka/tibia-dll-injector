#ifndef XTEA_H_
#define XTEA_H_

#include <cstdint>
#include <array>

class Xtea {
 public:
   static bool decrypt(const std::array<uint32_t, 4>& key, uint8_t *buffer, uint16_t length);
};

#endif  // XTEA_H_
