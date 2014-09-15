#ifndef BITS_H_
#define BITS_H_

#include <cstdint>
#include <array>

class Bits {
 public:

  template <std::size_t SIZE>
  static uint8_t getU8(const std::array<uint8_t, SIZE>& buffer,
                       std::size_t pos) {
    return buffer[pos];
  }

  template <std::size_t SIZE>
  static uint16_t getU16(const std::array<uint8_t, SIZE>& buffer,
                         std::size_t pos) {
    return  (uint16_t)buffer[pos] |
           ((uint16_t)buffer[pos + 1] << 8);
  }

  template <std::size_t SIZE>
  static uint32_t getU32(const std::array<uint8_t, SIZE>& buffer,
                         std::size_t pos) {
    return  (uint16_t)buffer[pos] |
           ((uint16_t)buffer[pos + 1] << 8) |
           ((uint16_t)buffer[pos + 2] << 16) |
           ((uint16_t)buffer[pos + 3] << 24);
  }

  template <std::size_t SIZE>
  static void addU8(std::array<uint8_t, SIZE>& buffer,
                    std::size_t pos,
                    uint8_t val) {
    buffer[pos] = val;
  }

  template <std::size_t SIZE>
  static void addU16(std::array<uint8_t, SIZE>& buffer,
                     std::size_t pos,
                     uint16_t val) {
    buffer[pos] = val & 0xFF;
    buffer[pos + 1] = (val >> 8) & 0xFF;
  }

  template <std::size_t SIZE>
  static void addU32(std::array<uint8_t, SIZE>& buffer,
                     std::size_t pos,
                     uint32_t val) {
    buffer[pos] = val & 0xFF;
    buffer[pos + 1] = (val >> 8) & 0xFF;
    buffer[pos + 2] = (val >> 16) & 0xFF;
    buffer[pos + 3] = (val >> 24) & 0xFF;
  }
};

#endif  // BITS_H_
