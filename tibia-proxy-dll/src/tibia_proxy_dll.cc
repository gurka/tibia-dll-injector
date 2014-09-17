#include <cstdint>
#include <array>
#include <windows.h>

#include "tibia_proxy_dll.h"
#include "packet.h"
#include "bits.h"
#include "dll_protocol.h"

// Handle to the thread
static HANDLE dllThread;

// Function pointers to WS2_32 functions
static RECV_PTR            ws2_32_recv            = (RECV_PTR)            GetProcAddress(GetModuleHandleA("WS2_32.dll"), "recv");
static SEND_PTR            ws2_32_send            = (SEND_PTR)            GetProcAddress(GetModuleHandleA("WS2_32.dll"), "send");
static SOCKET_PTR          ws2_32_socket          = (SOCKET_PTR)          GetProcAddress(GetModuleHandleA("WS2_32.dll"), "socket");
static HTONS_PTR           ws2_32_htons           = (HTONS_PTR)           GetProcAddress(GetModuleHandleA("WS2_32.dll"), "htons");
static INET_ADDR_PTR       ws2_32_inet_addr       = (INET_ADDR_PTR)       GetProcAddress(GetModuleHandleA("WS2_32.dll"), "inet_addr");
static CONNECT_PTR         ws2_32_connect         = (CONNECT_PTR)         GetProcAddress(GetModuleHandleA("WS2_32.dll"), "connect");
static CLOSESOCKET_PTR     ws2_32_closesocket     = (CLOSESOCKET_PTR)     GetProcAddress(GetModuleHandleA("WS2_32.dll"), "closesocket");
static WSAGETLASTERROR_PTR ws2_32_WSAGetLastError = (WSAGETLASTERROR_PTR) GetProcAddress(GetModuleHandleA("WS2_32.dll"), "WSAGetLastError");
static WSASETLASTERROR_PTR ws2_32_WSASetLastError = (WSASETLASTERROR_PTR) GetProcAddress(GetModuleHandleA("WS2_32.dll"), "WSASetLastError");

// Function pointers to our WS2_32 functions
static DWORD_PTR our_recv_ptr        = (DWORD_PTR) &our_recv;
static DWORD_PTR our_send_ptr        = (DWORD_PTR) &our_send;
static DWORD_PTR our_connect_ptr     = (DWORD_PTR) &our_connect;
static DWORD_PTR our_closesocket_ptr = (DWORD_PTR) &our_closesocket;

// The memory addresses which we want to replace (Tibia 10.54)
static DWORD_PTR tibiaSendFuncPtr        = 0x34C9AC;
static DWORD_PTR tibiaRecvFuncPtr        = 0x34C994;
static DWORD_PTR tibiaConnectFuncPtr     = 0x34C984;
static DWORD_PTR tibiaCloseSocketFuncPtr = 0x34C980;
static DWORD_PTR tibiaXteaKeyAddress     = 0x41B8D4;

// This is set to true after a call to myConnect has been made
// It is used to send the XTEA key on the next call to mySend
static bool firstPacket = false;

// Our socket and packet
static SOCKET dllSocket;
static Packet dllPacket;

int WINAPI our_recv(SOCKET s, char* buf, int len, int flags) {
  int bytesCount = ws2_32_recv(s, buf, len, flags);
  int wsaError = ws2_32_WSAGetLastError();

  dllPacket.reset();
  if (bytesCount == 0) {
    dllPacket.addU8(DllProtocol::SERVER_CLIENT_CLOSED);
    sendDllPacket();
  } else if (bytesCount == SOCKET_ERROR) {
    dllPacket.addU8(DllProtocol::SERVER_CLIENT_ERROR);
    sendDllPacket();
  } else {
    dllPacket.addU8(DllProtocol::SERVER_CLIENT_DATA);
    dllPacket.addBytes(&buf[0], &buf[bytesCount]);
    sendDllPacket();
  }

  ws2_32_WSASetLastError(wsaError);
  return bytesCount;
}

int WINAPI our_send(SOCKET s, char* buf, int len, int flags) {
  int bytesCount = ws2_32_send(s, buf, len, flags);
  int wsaError = ws2_32_WSAGetLastError();

  dllPacket.reset();
  if (firstPacket) {
    DWORD_PTR tibiaHandle = (DWORD_PTR)GetModuleHandle(NULL);
    uint8_t* xteaKeys = (uint8_t*)(tibiaHandle + tibiaXteaKeyAddress);

    dllPacket.addU8(DllProtocol::XTEA_KEY);
    dllPacket.addBytes(&xteaKeys[0], &xteaKeys[16]);
    sendDllPacket();
    firstPacket = false;
  }

  if (bytesCount == SOCKET_ERROR) {
    dllPacket.addU8(DllProtocol::CLIENT_SERVER_ERROR);
    sendDllPacket();
  } else {
    dllPacket.addU8(DllProtocol::CLIENT_SERVER_DATA);
    dllPacket.addBytes(&buf[0], &buf[bytesCount]);
    sendDllPacket();
  }

  ws2_32_WSASetLastError(wsaError);
  return bytesCount;
}

int WINAPI our_connect(SOCKET s, const struct sockaddr* name, int namelen) {
  int ret = ws2_32_connect(s, name, namelen);
  int wsaError = ws2_32_WSAGetLastError();

  dllPacket.reset();
  dllPacket.addU8(DllProtocol::CLIENT_SERVER_CONNECT);
  dllPacket.addU8((ret == SOCKET_ERROR) ? 0x00 : 0x01);
  sendDllPacket();

  // Send XTEA key on next send
  firstPacket = true;

  ws2_32_WSASetLastError(wsaError);
  return ret;
}

int WINAPI our_closesocket(SOCKET s) {
  int ret = ws2_32_closesocket(s);
  int wsaError = ws2_32_WSAGetLastError();

  dllPacket.reset();
  dllPacket.addU8(DllProtocol::CLIENT_SERVER_CLOSE_SOCKET);
  dllPacket.addU8((ret == SOCKET_ERROR) ? 0x00 : 0x01);
  sendDllPacket();

  ws2_32_WSASetLastError(wsaError);
  return ret;
}

void sendDllPacket() {
  // Send length
  std::array<uint8_t, 2> packetLength;
  Bits::addU16(packetLength, 0, dllPacket.length());
  int left = 2;
  while (left > 0) {
    int temp = ws2_32_send(dllSocket, (char*)(&packetLength[0]), left, 0);
    if (temp == SOCKET_ERROR) {
      return;  // TODO: Uninject self
    } else {
      left -= temp;
    }
  }

  // Send packet
  left = dllPacket.length();
  const uint8_t* buffer = dllPacket.getBuffer();
  while (left > 0) {
    int temp = ws2_32_send(dllSocket,
                           (char*)(&buffer[dllPacket.length() - left]),
                           left,
                           0);
    if (temp == SOCKET_ERROR) {
      return;  // TODO: Uninject self
    } else {
      left -= temp;
    }
  }
}

void dllThreadFunc(HMODULE dllModule) {
  // Create socket
  dllSocket = ws2_32_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (dllSocket == INVALID_SOCKET) {
    return;
  }

  // Connect to server
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = ws2_32_htons(8181);
  address.sin_addr.S_un.S_addr = ws2_32_inet_addr("127.0.0.1");
  if (ws2_32_connect(dllSocket, (sockaddr*)(&address), sizeof(address)) == SOCKET_ERROR) {
    ws2_32_closesocket(dllSocket);
    return;
  }

  DWORD dwOldProtect;
  DWORD dwNewProtect;

  // Get tibia handle and calculate addresses to send and recv funcs
  DWORD_PTR tibiaHandle = (DWORD_PTR)GetModuleHandle(NULL);
  DWORD_PTR realTibiaSendCallPtr        = tibiaHandle + tibiaSendFuncPtr;
  DWORD_PTR realTibiaRecvCallPtr        = tibiaHandle + tibiaRecvFuncPtr;
  DWORD_PTR realTibiaConnectCallPtr     = tibiaHandle + tibiaConnectFuncPtr;
  DWORD_PTR realTibiaCloseSocketCallPtr = tibiaHandle + tibiaCloseSocketFuncPtr;

  // Replace Tibias func pointers with our func pointers
  VirtualProtect((PVOID)realTibiaSendCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaSendCallPtr, &our_send_ptr, 4);
  VirtualProtect((PVOID)realTibiaSendCallPtr, 4, dwOldProtect, &dwNewProtect);

  VirtualProtect((PVOID)realTibiaRecvCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaRecvCallPtr, &our_recv_ptr, 4);
  VirtualProtect((PVOID)realTibiaRecvCallPtr, 4, dwOldProtect, &dwNewProtect);

  VirtualProtect((PVOID)realTibiaConnectCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaConnectCallPtr, &our_connect_ptr, 4);
  VirtualProtect((PVOID)realTibiaConnectCallPtr, 4, dwOldProtect, &dwNewProtect);

  VirtualProtect((PVOID)realTibiaCloseSocketCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaCloseSocketCallPtr, &our_closesocket_ptr, 4);
  VirtualProtect((PVOID)realTibiaCloseSocketCallPtr, 4, dwOldProtect, &dwNewProtect);
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      dllThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dllThreadFunc, hinstDLL, 0, NULL);
      break;
    case DLL_PROCESS_DETACH:
      TerminateThread(dllThread, EXIT_SUCCESS);
      break;
  }
  return TRUE;
}
