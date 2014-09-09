#include <algorithm>
#include "packet.h"
#include "bits.h"

Packet::Packet(const uint8_t* buffer, uint16_t length)
  : length_(length), pos_(0) {
  std::copy(&buffer[0], &buffer[length], &buffer_[0]);
}

Packet::Packet(const std::vector<uint8_t>& buffer)
  : length_(buffer.size()), pos_(0) {
  std::copy(buffer.cbegin(), buffer.cend(), &buffer_[0]);
}

uint8_t Packet::getU8() {
  uint8_t v = Bits::getU8(&buffer_[0], pos_);
  pos_ += 1;
  return v;
}

uint16_t Packet::getU16() {
  uint16_t v = Bits::getU16(&buffer_[0], pos_);
  pos_ += 2;
  return v;
}

uint32_t Packet::getU32() {
  uint32_t v = Bits::getU32(&buffer_[0], pos_);
  pos_ += 4;
  return v;
}

std::string Packet::getString() {
  uint16_t strlength = getU16();
  char* str = (char*)(&buffer_[pos_]);
  pos_ += strlength;
  return std::string(str, strlength);
}

std::vector<uint8_t> Packet::getBytes(uint8_t n) {
  std::vector<uint8_t> bytes(&buffer_[pos_], &buffer_[pos_ + n]);
  pos_ += n;
  return bytes;
}
