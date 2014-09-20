#include <utility>
#include <vector>
#include "market_data.h"
#include "packet.h"
#include "trace.h"

void MarketData::packetReceived(const Packet& packet, Direction direction) {
  Packet thePacket = packet;
  uint8_t opcode = thePacket.getU8();
  switch (opcode) {
    case 248: {
      parseMarketDetail(thePacket);
      break;
    case 249:
      parseMarketBrowse(thePacket);
      break;
    default:
      break;
    }
  }
}

void MarketData::parseMarketDetail(Packet& packet) {
  const std::vector<std::string> itemDescriptions = {
    { "Armor" },
    { "Attack" },
    { "Container" },
    { "Defense" },
    { "General" },
    { "DecayTime" },
    { "Combat" },
    { "MinLevel" },
    { "MinMagicLevel" },
    { "Vocation" },
    { "Rune" },
    { "Ability" },
    { "Charges" },
    { "WeaponName" },
    { "Weight" },
  };

  uint16_t itemId = packet.getU16();
  TRACE_INFO("itemId: %u", itemId);

  for (auto& itemDescription : itemDescriptions) {
    if (packet.peekU16() != 0x00) {
      TRACE_INFO("%s: %s",
                 itemDescription.c_str(),
                 packet.getString().c_str());
    } else {
      packet.getU16();
    }
  }
  /*
  uint8_t count = packet.getU8();
  for (int i = 0; i < count; i++) {
    uint32_t transactions = packet.getU32();
    uint32_t totalPrice = packet.getU32();
    uint32_t highestPrice = packet.getU32();
    uint32_t lowestPrice = packet.getU32();
    TRACE_INFO("Purchases stats: Transactions: %u Total price: %u "
               "Highest price: %u Lowest Price: %u",
               transactions, totalPrice, highestPrice, lowestPrice);
  }

  count = packet.getU8();
  for (int i = 0; i < count; i++) {
    uint32_t transactions = packet.getU32();
    uint32_t totalPrice = packet.getU32();
    uint32_t highestPrice = packet.getU32();
    uint32_t lowestPrice = packet.getU32();
    TRACE_INFO("Sales stats: Transactions: %u Total price: %u "
               "Highest price: %u Lowest Price: %u",
               transactions, totalPrice, highestPrice, lowestPrice);
  }
  */
}

void MarketData::parseMarketBrowse(Packet& packet) {
  uint16_t var = packet.getU16();

  uint32_t numBuyOffers = packet.getU32();
  for (unsigned int i = 0; i < numBuyOffers; i++) {
    uint32_t timestamp = packet.getU32();
    uint16_t count = packet.getU16();
    uint16_t itemId;
    if (var == 0xFFFE || var == 0xFFFF) {
      itemId = packet.getU16();
    } else {
      itemId = var;
    }
    uint16_t amount = packet.getU16();
    uint32_t price = packet.getU32();
    std::string playerName;
    uint8_t state = 0;
    if (var == 0xFFFF) {
      state = packet.getU8();
    } else {
      playerName = packet.getString();
    }

    TRACE_INFO("Buy offer: timestamp: %u count: %u itemId: %u amount %u "
               "price: %u playerName: %s state: %u",
               timestamp, count, itemId, amount,
               price, playerName.c_str(), state);
  }

  uint32_t numSellOffers = packet.getU32();
  for (unsigned int i = 0; i < numSellOffers; i++) {
    uint32_t timestamp = packet.getU32();
    uint16_t count = packet.getU16();
    uint16_t itemId;
    if (var == 0xFFFE || var == 0xFFFF) {
      itemId = packet.getU16();
    } else {
      itemId = var;
    }
    uint16_t amount = packet.getU16();
    uint32_t price = packet.getU32();
    std::string playerName;
    uint8_t state = 0;
    if (var == 0xFFFF) {
      state = packet.getU8();
    } else {
      playerName = packet.getString();
    }

    TRACE_INFO("Sell offer: timestamp: %u count: %u itemId: %u amount %u "
               "price: %u playerName: %s state: %u",
               timestamp, count, itemId, amount,
               price, playerName.c_str(), state);
  }
}
