#ifndef MAP_H_
#define MAP_H_

#include "module.h"

class Packet;

class Map : public Module {
 public:
  Map();

  void packetReceived(const Packet& packet, uint8_t opcode);
};

#endif  // MAP_H_
