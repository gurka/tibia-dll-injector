#include <iostream>
#include "packet_buffer.h"
#include "packet.h"
#include "bits.h"
#include "xtea.h"

PacketBuffer::PacketBuffer(std::function<void(Packet&)> onCompletePacket)
  : onCompletePacket_(onCompletePacket),
    position_(0) {
}

void PacketBuffer::checkCompletePackets() {
  // Do we have enough data to retrieve the packet length?
  while (position_ >= 2) {

    // Get packet length
    uint16_t packetLength = Bits::getU16(packetBuffer_, 0);
    if (2U + packetLength > position_) {
      break;  // We don't have a complete packet
    }

    // Decrypt buffer
    if (!Xtea::decrypt(xteaKey_, &packetBuffer_[6], packetLength - 4)) {
      std::cerr << "Could not decrypt packet" << std::endl;
    } else {
      // Get decrypted packet length
      uint16_t decrypted_length = Bits::getU32(packetBuffer_, 6);
      if (decrypted_length > packetLength) {
        std::cerr << "Decrypted length (" << decrypted_length << ")"
                  << " > encrypted length (" << (packetLength - 4) << ")"
                  << std::endl;
      } else {
        // Create packet and call our callback
        Packet packet(packetBuffer_.cbegin() + 8,
                      packetBuffer_.cbegin() + 8 + decrypted_length);
        onCompletePacket_(packet);
      }
    }

    // Do we have any more data, i.e. data for the next packet?
    uint16_t bytesLeft = position_ - (2 + packetLength);
    if (bytesLeft > 0) {
      // Yes: shift the data back to the beginning of the buffer
      std::copy(packetBuffer_.cbegin() + 2 + packetLength,
                packetBuffer_.cbegin() + 2 + packetLength + bytesLeft,
                packetBuffer_.begin());
    }
    position_ = bytesLeft;
  }
}
