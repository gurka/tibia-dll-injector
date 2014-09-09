#ifndef XTEA_H_
#define XTEA_H_

#include <cstdint>

class Xtea {
 public:
   static bool decrypt(const uint32_t key[4], uint8_t *buffer, uint16_t length);
};

#endif  // XTEA_H_
