#include <sstream>
#include <windows.h>
#include "tibia_proxy_dll.h"

// Handle to the thread
static HANDLE dllThread;

// Function pointers to WS2_32 functions
static PRECV        ws2_32_recv_ptr        = (PSEND)        GetProcAddress(GetModuleHandleA("WS2_32.dll"), "recv");
static PSEND        ws2_32_send_ptr        = (PRECV)        GetProcAddress(GetModuleHandleA("WS2_32.dll"), "send");
static PSOCKET      ws2_32_socket_ptr      = (PSOCKET)      GetProcAddress(GetModuleHandleA("WS2_32.dll"), "socket");
static PHTONS       ws2_32_htons_ptr       = (PHTONS)       GetProcAddress(GetModuleHandleA("WS2_32.dll"), "htons");
static PINET_ADDR   ws2_32_inet_addr_ptr   = (PINET_ADDR)   GetProcAddress(GetModuleHandleA("WS2_32.dll"), "inet_addr");
static PCONNECT     ws2_32_connect_ptr     = (PCONNECT)     GetProcAddress(GetModuleHandleA("WS2_32.dll"), "connect");
static PCLOSESOCKET ws2_32_closesocket_ptr = (PCLOSESOCKET) GetProcAddress(GetModuleHandleA("WS2_32.dll"), "closesocket");

static PWSAGETLASTERROR WSAGetLastError_ptr = (PWSAGETLASTERROR) GetProcAddress(GetModuleHandleA("WS2_32.dll"), "WSAGetLastError");
static PWSASETLASTERROR WSASetLastError_ptr = (PWSASETLASTERROR) GetProcAddress(GetModuleHandleA("WS2_32.dll"), "WSASetLastError");

// Function pointers to our WS2_32 functions
static DWORD_PTR myRecvPtr        = (DWORD_PTR)&myRecv;
static DWORD_PTR mySendPtr        = (DWORD_PTR)&mySend;
static DWORD_PTR myConnectPtr     = (DWORD_PTR)&myConnect;
static DWORD_PTR myCloseSocketPtr = (DWORD_PTR)&myCloseSocket;

// The memory addresses which we want to replace (Tibia 10.53)
static DWORD_PTR tibiaSendFuncPtr        = 0x3409AC;
static DWORD_PTR tibiaRecvFuncPtr        = 0x340994;
static DWORD_PTR tibiaConnectFuncPtr     = 0x340984;
static DWORD_PTR tibiaCloseSocketFuncPtr = 0x340980;
static DWORD_PTR tibiaXteaKeyAddress     = 0x40D734;

// This is set to true after a call to myConnect has been made
// It is used to send the XTEA key on the next call to mySend
static bool firstPacket = false;

// Our socket and buffer
static SOCKET dllSocket;
static char dllBuffer[1024 * 10];

int WINAPI myRecv(SOCKET s, char* buf, int len, int flags) {
  int bytesCount = ws2_32_recv_ptr(s, buf, len, flags);
  int wsa_error = WSAGetLastError_ptr();

  if (bytesCount == 0) {
    // Server -> Client closed
    dllBuffer[0] = 0x00;
    sendDllPacket(&dllBuffer[0], 1);
  } else if (bytesCount == SOCKET_ERROR) {
    // Server -> Client error
    dllBuffer[0] = 0x01;
    sendDllPacket(&dllBuffer[0], 1);
  } else {
    // Server -> Client data
    dllBuffer[0] = 0x02;
    memcpy(&dllBuffer[1], buf, bytesCount);
    sendDllPacket(&dllBuffer[0], 1 + bytesCount);
  }

  WSASetLastError_ptr(wsa_error);
  return bytesCount;
}

int WINAPI mySend(SOCKET s, char* buf, int len, int flags) {
  int bytesCount = ws2_32_send_ptr(s, buf, len, flags);
  int wsa_error = WSAGetLastError_ptr();

  if (firstPacket) {
    DWORD_PTR tibiaHandle = (DWORD_PTR)GetModuleHandle(NULL);
    DWORD_PTR realXteaKeyAddress = tibiaHandle + tibiaXteaKeyAddress;

    // Send XTEA Key
    dllBuffer[0] = 0x07;
    memcpy(&dllBuffer[1], (void*)realXteaKeyAddress, 16);
    sendDllPacket(&dllBuffer[0], 17);
    firstPacket = false;
  }

  if (bytesCount == SOCKET_ERROR) {
    // Client -> Server error
    dllBuffer[0] = 0x03;
    sendDllPacket(&dllBuffer[0], 1);
  } else {
    // Client -> Server data
    dllBuffer[0] = 0x04;
    memcpy(&dllBuffer[1], buf, bytesCount);
    sendDllPacket(&dllBuffer[0], 1 + bytesCount);
  }

  WSASetLastError_ptr(wsa_error);
  return bytesCount;
}

int WINAPI myConnect(SOCKET s, const struct sockaddr* name, int namelen) {
  int ret = ws2_32_connect_ptr(s, name, namelen);
  int wsa_error = WSAGetLastError_ptr();

  dllBuffer[0] = 0x05;  // Client -> Server connect
  dllBuffer[1] = (ret == SOCKET_ERROR) ? 0x00 : 0x01;
  sendDllPacket(&dllBuffer[0], 2);

  // Send XTEA key on next send
  firstPacket = true;

  WSASetLastError_ptr(wsa_error);
  return ret;
}

int WINAPI myCloseSocket(SOCKET s) {
  int ret = ws2_32_closesocket_ptr(s);
  int wsa_error = WSAGetLastError_ptr();

  dllBuffer[0] = 0x06;  // Client -> Server close socket
  dllBuffer[1] = (ret == SOCKET_ERROR) ? 0x00 : 0x01;
  sendDllPacket(&dllBuffer[0], 2);

  WSASetLastError_ptr(wsa_error);
  return ret;
}

void sendDllPacket(char* buf, int len) {
  // Send length
  char packet_length[2];
  packet_length[0] = len & 0xFF;
  packet_length[1] = (len >> 8) & 0xFF;
  int left = 2;
  while (left > 0) {
    int temp = ws2_32_send_ptr(dllSocket, packet_length, left, 0);
    if (temp == SOCKET_ERROR) {
      return;  // TODO: Uninject self
    } else {
      left -= temp;
    }
  }

  // Send packet
  left = len;
  while (left > 0) {
    int temp = ws2_32_send_ptr(dllSocket, &buf[len - left], left, 0);
    if (temp == SOCKET_ERROR) {
      return;  // TODO: Uninject self
    } else {
      left -= temp;
    }
  }
}

void dllThreadFunc(HMODULE dllModule) {
  // Create socket
  dllSocket = ws2_32_socket_ptr(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (dllSocket == INVALID_SOCKET) {
    MessageBox(NULL, "Could not create socket", "TibiaProxyDLL Error", MB_ICONERROR | MB_OK);
    return;
  }

  // Connect to server
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = ws2_32_htons_ptr(8181);
  address.sin_addr.S_un.S_addr = ws2_32_inet_addr_ptr("127.0.0.1");
  if (ws2_32_connect_ptr(dllSocket, (sockaddr*)(&address), sizeof(address)) == SOCKET_ERROR) {
    MessageBox(NULL, "Could not connect to server", "TibiaProxyDLL Error", MB_ICONERROR | MB_OK);
    ws2_32_closesocket_ptr(dllSocket);
    return;
  }

  DWORD dwOldProtect;
  DWORD dwNewProtect;

  // Get tibia handle and calculate addresses to send and recv funcs
  DWORD_PTR tibiaHandle = (DWORD_PTR)GetModuleHandle(NULL);
  DWORD_PTR realTibiaSendCallPtr = tibiaHandle + tibiaSendFuncPtr;
  DWORD_PTR realTibiaRecvCallPtr = tibiaHandle + tibiaRecvFuncPtr;
  DWORD_PTR realTibiaConnectCallPtr = tibiaHandle + tibiaConnectFuncPtr;
  DWORD_PTR realTibiaCloseSocketCallPtr = tibiaHandle + tibiaCloseSocketFuncPtr;

  // Replace Tibias func pointers with our func pointers
  VirtualProtect((PVOID)realTibiaSendCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaSendCallPtr, &mySendPtr, 4);
  VirtualProtect((PVOID)realTibiaSendCallPtr, 4, dwOldProtect, &dwNewProtect);

  VirtualProtect((PVOID)realTibiaRecvCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaRecvCallPtr, &myRecvPtr, 4);
  VirtualProtect((PVOID)realTibiaRecvCallPtr, 4, dwOldProtect, &dwNewProtect);

  VirtualProtect((PVOID)realTibiaConnectCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaConnectCallPtr, &myConnectPtr, 4);
  VirtualProtect((PVOID)realTibiaConnectCallPtr, 4, dwOldProtect, &dwNewProtect);

  VirtualProtect((PVOID)realTibiaCloseSocketCallPtr, 4, PAGE_READWRITE, &dwOldProtect);
  memcpy((PVOID)realTibiaCloseSocketCallPtr, &myCloseSocketPtr, 4);
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
