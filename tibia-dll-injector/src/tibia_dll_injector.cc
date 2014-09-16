#include <cstdio>
#include <cstdint>
#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <Winsock2.h>

#include "server.h"
#include "packet.h"
#include "bits.h"
#include "protocol.h"
#include "dll_protocol.h"
#include "packet_buffer.h"

// Path to the DLL to inject (hint: replace this string to reflect your location of the code/dll)
static char dllName[] = "C:\\Users\\gurka\\code\\tibia-dll-injector\\tibia-proxy-dll\\bin\\release\\tibia-proxy-dll.dll";

DWORD findTibiaPid() {
  // Enumerate all processes
  DWORD pids[1024];
  DWORD temp;
  if (!EnumProcesses(pids, sizeof(pids), &temp)) {
    return 1;
  }

  // Find the first process with the name "Tibia.exe"
  DWORD noPids = temp / sizeof(DWORD);
  for (DWORD i = 0; i < noPids; i++) {
    if (pids[i] == 0) {
      continue;
    }
    HANDLE tempHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pids[i]);
    if (tempHandle == NULL) {
      continue;
    }
    HMODULE tempModule;
    if (EnumProcessModules(tempHandle, &tempModule, sizeof(tempModule), &temp)) {
      TCHAR szProcessName[MAX_PATH];
      GetModuleBaseName(tempHandle, tempModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
      if (strcmp("Tibia.exe", szProcessName) == 0) {
        return pids[i];
      }
    }
  }
  return 0;
}

bool injectDLL(DWORD pid) {
  // Open process using pid
  HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
  if (handle == NULL) {
    return false;
  }

  // Get the address to the function LoadLibraryA in kernel32.dll
  LPVOID LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
  if (LoadLibAddr == NULL) {
    return false;
  }

  // Allocate memory inside the opened process
  LPVOID dereercomp = VirtualAllocEx(handle, NULL, strlen(dllName), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (dereercomp == NULL) {
    return false;
  }

  // Write the DLL name to the allocated memory
  if (!WriteProcessMemory(handle, dereercomp, dllName, strlen(dllName), NULL)) {
    return false;
  }

  // Create a thread in the opened process
  HANDLE remoteThread = CreateRemoteThread(handle, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibAddr, dereercomp, 0, NULL);
  if (remoteThread == NULL) {
    return false;
  }

  // Wait until thread have started (or stopped?)
  WaitForSingleObject(remoteThread, INFINITE);

  // Free the allocated memory
  VirtualFreeEx(handle, dereercomp, strlen(dllName), MEM_RELEASE);

  // Close the handles
  CloseHandle(remoteThread);
  CloseHandle(handle);

  return true;
}

void WSADeinit() {
  WSACleanup();

  // Ugly hack
  getchar();
}

bool WSAInit() {
  WSADATA wsaData;

  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    return false;
  }
  atexit(WSADeinit);

  return LOBYTE(wsaData.wVersion) == 2 && HIBYTE(wsaData.wVersion) == 2;
}

bool recvAll(SOCKET s, char* buf, int len) {
  int left = len;
  while (left > 0) {
    int temp = recv(s, buf + (len - left), left, 0);
    if (temp == SOCKET_ERROR || temp == 0) {
      return false;
    } else {
      left -= temp;
    }
  }
  return true;
}

void onServerToClientPacket(Packet& packet) {
  uint8_t opcode = packet.getU8();
  std::map<uint8_t, std::string>::iterator opcodeStringIt = Protocol::gameServerOpcodes.find(opcode);
  if (opcodeStringIt != Protocol::gameServerOpcodes.end()) {
    std::cout << "Server -> Client [" << opcodeStringIt->second << "]" << std::endl;
  } else {
    std::cout << "Server -> Client <" << (uint16_t)opcode << ">" << std::endl;
  }
}

void onClientToServerPacket(Packet& packet) {
  uint8_t opcode = packet.getU8();
  std::map<uint8_t, std::string>::iterator opcodeStringIt = Protocol::clientOpcodes.find(opcode);
  if (opcodeStringIt != Protocol::gameServerOpcodes.end()) {
    std::cout << "Client -> Server [" << opcodeStringIt->second << "]" << std::endl;
  } else {
    std::cout << "Client -> Server <" << (uint16_t)opcode << ">" << std::endl;
  }
}

bool receiveDllPacket(SOCKET clientSocket, Packet& dllPacket) {
  // Get DLL packet length
  std::array<uint8_t, 2> dllPacketLength;
  if (!recvAll(clientSocket, (char*)dllPacketLength.data(), 2)) {
    return false;
  }
  uint16_t dll_packet_length = Bits::getU16(dllPacketLength, 0);

  // Get DLL packet
  if (!recvAll(clientSocket, (char*)dllPacket.getBuffer(), dll_packet_length)) {
    return false;
  }
  dllPacket.reset(dll_packet_length);

  return true;
}

void receiveLoop(SOCKET clientSocket) {
  Packet dllPacket;
  PacketBuffer serverToClientPB(onServerToClientPacket);
  PacketBuffer clientToServerPB(onClientToServerPacket);

  while (true) {
    if (!receiveDllPacket(clientSocket, dllPacket)) {
      std::cerr << "Connection to DLL closed" << std::endl;
      break;
    }

    uint8_t packet_type = dllPacket.getU8();
    switch (packet_type) {
      case DllProtocol::SERVER_CLIENT_CLOSED: {
        std::cout << "Server -> Client closed" << std::endl;
        break;
      }
      case DllProtocol::SERVER_CLIENT_ERROR: {
        std::cout << "Server -> Client error" << std::endl;
        break;
       }
      case DllProtocol::SERVER_CLIENT_DATA: {
          std::cout << "Server -> Client data" << std::endl;
          std::size_t numBytes = dllPacket.bytesLeftReadable();
          std::vector<uint8_t> data = dllPacket.getBytes(numBytes);
          serverToClientPB.addToBuffer(data.cbegin(), data.cend());
          break;
      }
      case DllProtocol::CLIENT_SERVER_ERROR: {
        std::cout << "Client -> Server error" << std::endl;
        break;
      }
      case DllProtocol::CLIENT_SERVER_DATA: {
          std::cout << "Client -> Server data" << std::endl;
          std::size_t numBytes = dllPacket.bytesLeftReadable();
          std::vector<uint8_t> data = dllPacket.getBytes(numBytes);
          clientToServerPB.addToBuffer(data.cbegin(), data.cend());
          break;
      }
      case DllProtocol::CLIENT_SERVER_CONNECT: {
        std::cout << "Client -> Server connect"
                  << "(" << (dllPacket.getU8() == 0x00 ? "NOK" : "OK") << ")"
                  << std::endl;
        serverToClientPB.resetPosition();
        clientToServerPB.resetPosition();
        break;
      }
      case DllProtocol::CLIENT_SERVER_CLOSE_SOCKET: {
        std::cout << "Client -> Server close socket"
                  << "(" << (dllPacket.getU8() == 0x00 ? "NOK" : "OK") << ")"
                  << std::endl;
        break;
      }
      case DllProtocol::XTEA_KEY: {
        std::cout << "XTEA Key" << std::endl;
        std::array<uint32_t, 4> xteaKey;
        for (int i = 0; i < 4; i++) {
          xteaKey[i] = dllPacket.getU32();
        }
        serverToClientPB.setXteaKey(xteaKey);
        clientToServerPB.setXteaKey(xteaKey);
        break;
      }
      default: {
        std::cerr << "Unknown DLL packet type: " << (uint16_t)packet_type << std::endl;
        break;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  std::cout << "Initalizing WSA" << std::endl;
  if (!WSAInit()) {
    std::cerr << "Could not initialize WSA" << std::endl;
    return 1;
  }

  std::cout << "Starting server" << std::endl;
  Server server = Server("8181");
  if (!server.setup()) {
    std::cerr << "Could not setup server" << std::endl;
  }

  std::cout << "Finding Tibia pid" << std::endl;
  DWORD tibiaPid = findTibiaPid();
  if (tibiaPid == 0) {
    std::cerr << "Could not find Tibia process" << std::endl;
    return 1;
  }

  std::cout << "Injecting DLL" << std::endl;
  if (!injectDLL(tibiaPid)) {
    std::cerr << "Could not inject DLL" << std::endl;
    return 1;
  }

  std::cout << "Waiting for DLL to connect" << std::endl;
  SOCKET clientSocket = server.accept();
  if (clientSocket == INVALID_SOCKET) {
    std::cerr << "Could not accept client connection" << std::endl;
    return 1;
  }

  std::cout << "Starting recv loop" << std::endl;
  receiveLoop(clientSocket);

  return 0;
}
