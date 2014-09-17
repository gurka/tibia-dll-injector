#include <utility>
#include "market_data.h"
#include "packet.h"
#include "trace.h"

MarketData::MarketData() {
  wantedPackets_.push_back(std::make_pair(Module::ServerToClient, 248));
}

void MarketData::packetReceived(const Packet& packet, uint8_t opcode) {
  Packet marketDataPacket = packet;
  TRACE_INFO("RECEIVED MARKET DATA PACKET WITH LENGTH %u!",
             marketDataPacket.bytesLeftReadable());
}
