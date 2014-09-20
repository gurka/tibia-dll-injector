#ifndef PACKET_DUMP_H
#define PACKET_DUMP_H

#include "module.h"

class PacketDump : public Module {
 public:
  void packetReceived(const Packet& packet, Direction direction);
};

#endif  // PACKET_DUMP_H
