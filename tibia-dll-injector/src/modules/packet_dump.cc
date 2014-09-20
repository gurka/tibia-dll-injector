#include <iostream>
#include <iomanip>
#include <sstream>
#include "packet_dump.h"
#include "packet.h"
#include "trace.h"

void PacketDump::packetReceived(const Packet& packet, PacketDirection direction) {
  Packet thePacket = packet;
  std::stringstream out;
  out << std::hex << std::setfill('0');

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
        out << std::setw(2) << (uint16_t) buf[i];
      } else {
        out << "  ";
      }
      if (i % 2 != 0) {
        out << ' ';
      }
    }

    out << "  '";
    for (int i = 0; i < 16; i++) {
      if (i < nread) {
        if (buf[i] < 32) {
          out << '.';
        } else {
          out << (uint8_t) buf[i];
        }
      } else {
        out << ' ';
      }
    }
    out << '\'';

    std::cout << out.str() << std::endl;
    out.str("");
  }
}
