#ifndef MARKET_DATA_H_
#define MARKET_DATA_H_

#include "module.h"

class Packet;

class MarketData : public Module {
 public:
  MarketData();

  void packetReceived(const Packet& packet, uint8_t opcode);
};

#endif  // MARKET_DATA_H_
