#include <iostream>
#include <iomanip>
#include "packet_dump.h"
#include "packet.h"
#include "trace.h"

void PacketDump::packetReceived(const Packet& packet, Direction direction) {
  Packet thePacket = packet;
  std::cout << std::hex << std::setfill('0');

  while (thePacket.bytesLeftReadable() > 0) {

    int nread;
    uint8_t buf[16];
    for (nread = 0; nread < 16; nread++) {
      buf[nread] = thePacket.getU8();
      if (thePacket.bytesLeftReadable() == 0) {
        break;
      }
    }

    for (int i = 0; i < 16; i++) {
      if (i < nread) {
        std::cout << ' ' << std::setw(2) << (uint16_t) buf[i];
      } else {
        std::cout << "   ";
      }
    }

    std::cout << "  ";
    for (int i = 0; i < nread; i++) {
      if (buf[i] < 32) {
        std::cout << '.';
      } else {
        std::cout << (uint8_t) buf[i];
      }
    }
  }
}
