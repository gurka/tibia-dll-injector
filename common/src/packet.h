#ifndef PACKET_H_
#define PACKET_H_

#include <cstdint>
#include <array>
#include <string>
#include <vector>

class Packet {
 public:
  Packet() : writePos_(0), readPos_(0) {}

  template <class InputIterator>
  Packet(InputIterator first, InputIterator last)
    : writePos_(std::distance(first, last)), readPos_(0) {
    std::copy(first, last, buffer_.begin());
  }

  void reset() { writePos_ = 0; readPos_ = 0; }
  void reset(uint16_t length) { writePos_ = length; readPos_ = 0; }
  std::size_t length() const { return writePos_; }

  const uint8_t* getBuffer() const { return buffer_.data(); }

  void addU8(uint8_t v);
  void addU16(uint16_t v);
  void addU32(uint32_t v);
  void addString(const std::string& str);

  template <class InputIterator>
  void addBytes(InputIterator first, InputIterator last) {
    std::copy(first, last, buffer_.begin() + writePos_);
    writePos_ += std::distance(first, last);
  }

  void fillBytes(uint8_t v, std::size_t n);
  std::size_t bytesLeftWritable() const { return buffer_.size() - writePos_; }

  uint8_t getU8();
  uint16_t getU16();
  uint32_t getU32();
  std::string getString();
  std::vector<uint8_t> getBytes(uint16_t n);
  std::vector<uint8_t> getAllBytes() { return getBytes(bytesLeftReadable()); }
  void skipBytes(std::size_t n) { readPos_ += n; }
  std::size_t bytesLeftReadable() const { return writePos_ - readPos_; }

 private:
  std::array<uint8_t, 1024 * 16> buffer_;
  std::size_t writePos_;
  std::size_t readPos_;
};

#endif  // PACKET_H_
