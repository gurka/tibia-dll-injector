#include <utility>
#include "map.h"
#include "packet.h"
#include "trace.h"

void Map::packetReceived(const Packet& packet, PacketDirection direction) {
  Packet mapPacket = packet;
  uint8_t opcode = mapPacket.getU8();
  switch (opcode) {
    case 100: {
      TRACE_INFO("RECEIVED FULL MAP PACKET WITH LENGTH %u!",
                 mapPacket.bytesLeftReadable());
      break;
    }
    case 101: {
      TRACE_INFO("RECEIVED TOP ROW MAP PACKET WITH LENGTH %u!",
                 mapPacket.bytesLeftReadable());
      break;
    }
    case 102: {
      TRACE_INFO("RECEIVED RIGHT COLUMN MAP PACKET WITH LENGTH %u!",
                 mapPacket.bytesLeftReadable());
      break;
    }
    case 103: {
      TRACE_INFO("RECEIVED BOTTOM ROW MAP PACKET WITH LENGTH %u!",
                 mapPacket.bytesLeftReadable());
      break;
    }
    case 104: {
      TRACE_INFO("RECEIVED LEFT COLUMN MAP PACKET WITH LENGTH %u!",
                 mapPacket.bytesLeftReadable());
      break;
    default:
      break;
    }
  }
}

