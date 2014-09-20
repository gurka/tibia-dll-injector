#ifndef MODULE_H_
#define MODULE_H_

#include <cstdint>
#include <vector>

class Packet;

class Module {
 public:
  enum Direction {
    ServerToClient,
    ClientToServer,
  };

  virtual ~Module() {}

  virtual void packetReceived(const Packet& packet, Direction direction) = 0;
};

#endif  // MODULE_H_
