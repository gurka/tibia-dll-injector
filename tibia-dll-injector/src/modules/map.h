#ifndef MAP_H_
#define MAP_H_

#include "module.h"

class Map : public Module {
 public:
  void packetReceived(const Packet& packet, Direction direction);
};

#endif  // MAP_H_
