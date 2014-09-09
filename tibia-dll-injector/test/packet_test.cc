#include <cassert>
#include <iostream>
#include "../src/packet.h"
#include "../src/rsa.h"

/// Test Packet::getAllBytes
void test_1() {
  uint8_t buffer[] = {
    0x64,                   // 0d100
    0xDE, 0xAD, 0xBA, 0xBE, // 0d3199905246
    0x39, 0x05,             // 0d1337
  };

  Packet p(buffer, sizeof(buffer));
  std::vector<uint8_t> packetBytes = p.getAllBytes();
  for (unsigned int i = 0; i < sizeof(buffer); i++) {
    assert(buffer[i] == packetBytes.at(i));
  }
}

/// Test Packet::getU8, getU16, getU32
void test_2() {
  uint8_t buffer[] = {
    0x64,                   // 0d100
    0xDE, 0xAD, 0xBA, 0xBE, // 0d3199905246
    0x39, 0x05,             // 0d1337
  };

  Packet p(buffer, sizeof(buffer));
  assert(p.getU8() == 100);
  assert(p.getU32() == 3199905246);
  assert(p.getU16() == 1337);
}

/// Test Packet::getString
void test_3() {
  uint8_t buffer[] = {
    0x03, 0x00,       // string length = 3
    0x41, 0x42, 0x43  // string 'ABC'
  };

  Packet p(buffer, sizeof(buffer));
  assert(p.getString() == "ABC");
}

/// Test OT login packet (TODO: Testing RSA doesn't belong here)
void test_4() {
  const char* p("14299623962416399520070177382898895550795403345466153217470516082934737582776038882967213386204600674145392845853859217990626450972452084065728686565928113");
	const char* q("7630979195970404721891201847792002125535401292779123937207447574596692788513647179235335529307251350570728407373705564708871762033017096809910315212884101");

	uint8_t buffer[] = {
    0x9a, 0x00,              // Packet length
    0x69, 0x46, 0x13, 0x4c,  // Checksum
    0x01,                    // 0x01 == login packet
    0x02, 0x00,              // OS
    0x1d, 0x04,              // Version
    0x1d, 0x04, 0x00, 0x00,  // Protocol version
    0x6e, 0xd7, 0xfa, 0x53,  // .dat signature
    0x99, 0xd7, 0xfa, 0x53,  // .spr signature
    0x01, 0xa1, 0xba, 0x53,  // .pic signature
    0x00,                    // zero
    0x7a, 0x0d, 0x41, 0xac, 0xf1, 0x6a, 0xa4, 0x43, 0x63, 0xdc, 0xb1, 0xab, 0x28, 0x2b, 0xd8, 0x6d, 0x70, 0x3a, 0x66, 0x25, 0xc2, 0x60, 0xe7, 0x7e, 0xf2, 0xc0, 0x97, 0x89, 0x87, 0xb6, 0x7c, 0x8b, 0x19, 0x26, 0x4a, 0xc3, 0x9b, 0xbd, 0x7c, 0xf0, 0x2e, 0xcf, 0x6e, 0xad, 0xca, 0xf1, 0x09, 0x1a, 0x42, 0x23, 0x83, 0xf3, 0x75, 0x65, 0x0b, 0x40, 0x6b, 0xe8, 0xdb, 0x0b, 0xce, 0x9c, 0x52, 0x22, 0x7e, 0xb9, 0xa3, 0xc1, 0xbe, 0xa6, 0x5e, 0xfe, 0xcc, 0x98, 0x96, 0xed, 0x37, 0x5f, 0x26, 0xa7, 0x23, 0x4f, 0x62, 0x35, 0x8d, 0xe0, 0xf3, 0xed, 0xd8, 0xba, 0x84, 0x61, 0x62, 0x09, 0x5f, 0x4d, 0x07, 0xa6, 0x84, 0x4c, 0x00, 0xb5, 0xf1, 0xd0, 0x5c, 0xfe, 0x7e, 0xba, 0x92, 0xa5, 0x1b, 0xe0, 0x11, 0x6e, 0x5d, 0x28, 0x0b, 0x0c, 0x89, 0x90, 0xc1, 0x26, 0x50, 0x14, 0x0b, 0x6c, 0x4e, 0xc9
	};

  uint16_t packet_length = (uint16_t)buffer[0] | ((uint16_t)(buffer[1]) << 8);
  Packet packet(buffer + 2, packet_length);
  packet.skipBytes(26);
  std::vector<uint8_t> encrypted = packet.getAllBytes();
  assert(encrypted.size() == 128);

  RSA rsa;
  rsa.setPrivateKey(p, q);
  rsa.decrypt((char*)encrypted.data());

  Packet login(encrypted);
  std::cout << (uint16_t)login.getU8() << std::endl;
  uint32_t xtea_key[4];
  xtea_key[0] = login.getU32();
  xtea_key[1] = login.getU32();
  xtea_key[2] = login.getU32();
  xtea_key[3] = login.getU32();

  std::string accountName = login.getString();
  std::string password = login.getString();

  for (int i = 0; i < 4; i++) {
    std::cout << "XTEA KEY[" << i << "]: " << xtea_key[i] << std::endl;
  }

  std::cout << "Account name: " << accountName << std::endl;
  std::cout << "Password: " << password << std::endl;
}

int main(int argc, char* argv[]) {
  test_1();
  test_2();
  test_3();
  test_4();

  return 0;
}
