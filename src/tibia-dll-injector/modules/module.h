#ifndef MODULE_H_
#define MODULE_H_

#include <cstdint>
#include <vector>

#include "packet.h"

class Module {
 public:
  virtual ~Module() {}

  enum PacketDirection {
    ServerToClient,
    ClientToServer,
  };

  virtual void packetReceived(const Packet& packet, PacketDirection direction) = 0;
};

#endif  // MODULE_H_
