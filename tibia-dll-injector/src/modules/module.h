#ifndef MODULE_H_
#define MODULE_H_

#include <cstdint>
#include <vector>

class Packet;

class Module {
 public:
  virtual ~Module() {}

  enum Direction {
    ServerToClient,
    ClientToServer,
  };

  const std::vector<std::pair<Direction, uint8_t>>& wantedPackets() {
    return wantedPackets_;
  }

  virtual void packetReceived(const Packet& packet, uint8_t opcode) = 0;

 protected:
  std::vector<std::pair<Direction, uint8_t>> wantedPackets_;
};

#endif  // MODULE_H_
