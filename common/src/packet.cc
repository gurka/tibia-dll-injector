#include "packet.h"
#include "bits.h"

void Packet::addU8(uint8_t v) {
  Bits::addU8(buffer_, writePos_, v);
  writePos_ += 1;
}

void Packet::addU16(uint16_t v) {
  Bits::addU16(buffer_, writePos_, v);
  writePos_ += 2;
}

void Packet::addU32(uint32_t v) {
  Bits::addU32(buffer_, writePos_, v);
  writePos_ += 4;
}

void Packet::addString(const std::string& str) {
  addU16(str.size());
  std::copy(str.cbegin(),
            str.cend(),
            buffer_.begin() + writePos_);
  writePos_ += str.size();
}

void Packet::fillBytes(uint8_t v, std::size_t n) {
  std::fill(buffer_.begin() + writePos_,
            buffer_.begin() + writePos_ + n,
            v);
  writePos_ += n;
}

uint8_t Packet::getU8() {
  uint8_t v = Bits::getU8(buffer_, readPos_);
  readPos_ += 1;
  return v;
}

uint16_t Packet::getU16() {
  uint16_t v = Bits::getU16(buffer_, readPos_);
  readPos_ += 2;
  return v;
}

uint32_t Packet::getU32() {
  uint32_t v = Bits::getU32(buffer_, readPos_);
  readPos_ += 4;
  return v;
}

std::string Packet::getString() {
  uint16_t strlen = getU16();
  std::string str(buffer_.cbegin() + readPos_,
                  buffer_.cbegin() + readPos_ + strlen);
  readPos_ += strlen;
  return str;
}

std::vector<uint8_t> Packet::getBytes(uint16_t n) {
  std::vector<uint8_t> bytes(n);
  std::copy(buffer_.cbegin() + readPos_,
            buffer_.cbegin() + readPos_ + n,
            bytes.begin());
  readPos_ += n;
  return bytes;
}
