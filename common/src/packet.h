#ifndef PACKET_H_
#define PACKET_H_

#include <cstdint>
#include <array>
#include <string>
#include <vector>

class Packet {
 public:
  Packet(const uint8_t* buffer, uint16_t length);
  explicit Packet(const std::vector<uint8_t>& buffer);

  uint8_t bytesLeft() const { return length_ - pos_; }

  void skipBytes(uint8_t n) { pos_ += n; }
  uint8_t getU8();
  uint16_t getU16();
  uint32_t getU32();
  std::string getString();
  std::vector<uint8_t> getBytes(uint8_t n);
  std::vector<uint8_t> getAllBytes() { return getBytes(bytesLeft()); }

 private:
  std::array<uint8_t, 1024 * 16> buffer_;
  uint16_t length_;
  uint8_t pos_;
};

#endif  // PACKET_H_
