#ifndef PACKET_BUFFER_H_
#define PACKET_BUFFER_H_

#include <functional>

// Forward declaration
class Packet;

class PacketBuffer {
 public:
  PacketBuffer(std::function<void(Packet&)> onCompletePacket);

  template <class InputIterator>
  void addToBuffer(InputIterator first, InputIterator last) {
    // Copy data to our buffer
    std::copy(first, last, packetBuffer_.begin() + position_);
    position_ += std::distance(first, last);

    // Check if there is a complete packet received
    checkCompletePackets();
  }

  void setXteaKey(const std::array<uint32_t, 4>& xteaKey) {
    xteaKey_ = xteaKey;
  }
  void resetPosition() { position_ = 0; }

 private:
  std::function<void(Packet&)> onCompletePacket_;
  std::array<uint8_t, 1024 * 16> packetBuffer_;
  std::size_t position_;
  std::array<uint32_t, 4> xteaKey_;

  void checkCompletePackets();
};

#endif  // PACKET_BUFFER_H_
