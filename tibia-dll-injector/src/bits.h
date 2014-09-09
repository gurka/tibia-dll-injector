#ifndef BITS_H_
#define BITS_H_

class Bits {
 public:
  static uint8_t getU8(const uint8_t* buffer, size_t pos) {
    return buffer[pos];
  }

  static uint16_t getU16(const uint8_t* buffer, size_t pos) {
    return  (uint16_t)buffer[pos] |
           ((uint16_t)buffer[pos + 1] << 8);
  }

  static uint32_t getU32(const uint8_t* buffer, size_t pos) {
    return  (uint16_t)buffer[pos] |
           ((uint16_t)buffer[pos + 1] << 8) |
           ((uint16_t)buffer[pos + 2] << 16) |
           ((uint16_t)buffer[pos + 3] << 24);
  }

  static void addU8(uint8_t* buffer, size_t pos, uint8_t val) {
    buffer[pos] = val;
  }

  static void addU16(uint8_t* buffer, size_t pos, uint16_t val) {
    buffer[pos] = val & 0xFF;
    buffer[pos + 1] = (val >> 8) & 0xFF;
  }

  static void addU32(uint8_t* buffer, size_t pos, uint32_t val) {
    buffer[pos] = val & 0xFF;
    buffer[pos + 1] = (val >> 8) & 0xFF;
    buffer[pos + 2] = (val >> 16) & 0xFF;
    buffer[pos + 3] = (val >> 24) & 0xFF;
  }
};

#endif  // BITS_H_
