#ifndef MARKET_DATA_H_
#define MARKET_DATA_H_

#include "module.h"

class MarketData : public Module {
 public:
  void packetReceived(const Packet& packet, PacketDirection direction);

 private:
  void parseMarketDetail(Packet& packet);
  void parseMarketBrowse(Packet& packet);
};

#endif  // MARKET_DATA_H_
