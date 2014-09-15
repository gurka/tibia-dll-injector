#ifndef DLL_PROTOCOL_H_
#define DLL_PROTOCOL_H_

namespace DllProtocol {
  enum : uint8_t {
    SERVER_CLIENT_CLOSED       = 0x00,
    SERVER_CLIENT_ERROR        = 0x01,
    SERVER_CLIENT_DATA         = 0x02,
    CLIENT_SERVER_ERROR        = 0x03,
    CLIENT_SERVER_DATA         = 0x04,
    CLIENT_SERVER_CONNECT      = 0x05,
    CLIENT_SERVER_CLOSE_SOCKET = 0x06,
    XTEA_KEY                   = 0x07,
  };
}

#endif  // DLL_PROTOCOL_H_
