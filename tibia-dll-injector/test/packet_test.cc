#include <cassert>
#include <iostream>
#include "../src/packet.h"

bool test_readPacket() {
  std::vector<uint8_t> buffer = {
    0x64,                   // 0d100
    0x03, 0x00,             // string length = 3
    0x41, 0x42, 0x43,       // string "ABC"
    0xDE, 0xAD, 0xBA, 0xBE, // 0d3199905246
    0x00, 0x00, 0x00, 0x00, // 4 zero bytes
    0x00, 0x00, 0x00,       // 3 zero bytes
    0x39, 0x05,             // 0d1337
  };

  Packet packet(buffer.cbegin(), buffer.cend());
  assert(buffer.size() == packet.length());
  std::size_t left = packet.bytesLeftReadable();

  assert(packet.getU8() == 100);
  left -= 1;
  assert(left == packet.bytesLeftReadable());

  assert(packet.getString() == "ABC");
  left -= (2 + 3);
  assert(left == packet.bytesLeftReadable());

  assert(packet.getU32() == 3199905246);
  left -= 4;
  assert(left == packet.bytesLeftReadable());

  packet.skipBytes(4 + 3);
  left -= (4 + 3);
  assert(left == packet.bytesLeftReadable());

  assert(packet.getU16() == 1337);
  left -= 2;
  assert(left == packet.bytesLeftReadable());
  assert(left == 0);

  assert(buffer.size() == packet.length());

  return true;
}

bool test_writePacket() {
  Packet packet;
  std::size_t length = 0;
  assert(packet.length() == length);

  packet.addU8(100);
  length += 1;
  assert(packet.length() == length);

  packet.addString("ABC");
  length += (2 + 3);
  assert(packet.length() == length);

  packet.addU32(3199905246);
  length += 4;
  assert(packet.length() == length);

  packet.fillBytes(0x00, 7);
  length += 7;
  assert(packet.length() == length);

  packet.addU16(1337);
  length += 2;
  assert(packet.length() == length);

  std::vector<uint8_t> buffer = {
    0x64,                   // 0d100
    0x03, 0x00,             // string length = 3
    0x41, 0x42, 0x43,       // string "ABC"
    0xDE, 0xAD, 0xBA, 0xBE, // 0d3199905246
    0x00, 0x00, 0x00, 0x00, // 4 zero bytes
    0x00, 0x00, 0x00,       // 3 zero bytes
    0x39, 0x05,             // 0d1337
  };
  const uint8_t* packet_buffer = packet.getBuffer();

  for (std::size_t i = 0; i < packet.length(); i++) {
    assert((uint16_t)buffer[i] == (uint16_t)packet_buffer[i]);
  }

  return true;
}

bool test_writeAndReadPacket() {
  Packet packet;
  packet.addU16(5423);
  packet.addU16(1334);
  packet.addU8(5);
  packet.addString("Some random words and stuff! !\"#¤%&/()=?åäöÅÄÖ");
  packet.fillBytes(0x5F, 16);
  packet.addString("END");

  assert(74 == packet.bytesLeftReadable());

  assert(5423 ==  packet.getU16());
  assert(1334 == packet.getU16());
  assert(5 == packet.getU8());
  assert("Some random words and stuff! !\"#¤%&/()=?åäöÅÄÖ" == packet.getString());
  packet.skipBytes(16);
  assert("END" == packet.getString());

  assert(0 == packet.bytesLeftReadable());

  return true;
}

int main(int argc, char* argv[]) {
  test_readPacket();
  test_writePacket();
  test_writeAndReadPacket();

  return 0;
}
