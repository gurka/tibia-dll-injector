#include <utility>
#include "map.h"
#include "packet.h"
#include "trace.h"

Map::Map() {
  // GameServerFullMap
  wantedPackets_.push_back(std::make_pair(Module::ServerToClient, 100));
  // GameServerMapTopRow
  wantedPackets_.push_back(std::make_pair(Module::ServerToClient, 101));
  // GameServerMapRightRow
  wantedPackets_.push_back(std::make_pair(Module::ServerToClient, 102));
  // GameServerMapBottomRow
  wantedPackets_.push_back(std::make_pair(Module::ServerToClient, 103));
  //GameServerMapLeftRow
  wantedPackets_.push_back(std::make_pair(Module::ServerToClient, 104));
}

void Map::packetReceived(const Packet& packet, uint8_t opcode) {
  Packet mapPacket = packet;
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
      TRACE_ERROR("RECEIVED INVALID MAP PACKET");
    }
  }
}

