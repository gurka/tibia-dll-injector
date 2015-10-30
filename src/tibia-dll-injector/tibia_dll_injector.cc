#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <windows.h>
#include <psapi.h>
#include <Winsock2.h>

#include "server.h"
#include "packet.h"
#include "bits.h"
#include "protocol.h"
#include "dll_protocol.h"
#include "packet_buffer.h"
#include "trace.h"

#include "modules/module.h"
#include "modules/market_data.h"
#include "modules/map.h"
#include "modules/packet_dump.h"

// Path to the DLL to inject (hint: replace this string to reflect your location of the code/dll)
static char dllName[] = "libtibia-proxy-dll.dll";

static std::vector<Module*> modules;

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
  TRACE_INFO("Press any key to quit");
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
  uint8_t opcode = packet.peekU8();
  std::map<uint8_t, std::string>::iterator opcodeStringIt = Protocol::gameServerOpcodes.find(opcode);
  if (opcodeStringIt != Protocol::gameServerOpcodes.end()) {
    TRACE_INFO("Server -> Client [%s]", opcodeStringIt->second.c_str());
  } else {
    TRACE_INFO("Server -> Client <0x%x>", opcode);
  }

  for (Module* module : modules) {
    module->packetReceived(packet, Module::ServerToClient);
  }
}

void onClientToServerPacket(Packet& packet) {
  uint8_t opcode = packet.peekU8();
  std::map<uint8_t, std::string>::iterator opcodeStringIt = Protocol::clientOpcodes.find(opcode);
  if (opcodeStringIt != Protocol::gameServerOpcodes.end()) {
    TRACE_INFO("Client -> Server [%s]", opcodeStringIt->second.c_str());
  } else {
    TRACE_INFO("Client -> Server <0x%x>", opcode);
  }

  for (Module* module : modules) {
    module->packetReceived(packet, Module::ClientToServer);
  }
}

bool receiveDllPacket(SOCKET clientSocket, Packet& dllPacket) {
  // Get DLL packet length
  std::array<uint8_t, 2> dllPacketLength;
  if (!recvAll(clientSocket, (char*)dllPacketLength.data(), 2)) {
    return false;
  }
  uint16_t dll_packet_length = Bits::getU16(dllPacketLength, 0);
  TRACE_DEBUG("Received dllPacket length: %u", dll_packet_length);

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
      TRACE_ERROR("Connection to DLL closed");
      break;
    }

    uint8_t packet_type = dllPacket.getU8();
    switch (packet_type) {
      case DllProtocol::SERVER_CLIENT_CLOSED: {
        TRACE_INFO("Server -> Client closed");
        break;
      }
      case DllProtocol::SERVER_CLIENT_ERROR: {
        TRACE_INFO("Server -> Client error");
        break;
       }
      case DllProtocol::SERVER_CLIENT_DATA: {
        std::size_t numBytes = dllPacket.bytesLeftReadable();
        TRACE_DEBUG("Server -> Client data (%u bytes)", numBytes);
        std::vector<uint8_t> data = dllPacket.getBytes(numBytes);
        serverToClientPB.addToBuffer(data.cbegin(), data.cend());
        break;
      }
      case DllProtocol::CLIENT_SERVER_ERROR: {
        TRACE_INFO("Client -> Server error");
        break;
      }
      case DllProtocol::CLIENT_SERVER_DATA: {
        std::size_t numBytes = dllPacket.bytesLeftReadable();
        TRACE_DEBUG("Client -> Server data (%u bytes)", numBytes);
        std::vector<uint8_t> data = dllPacket.getBytes(numBytes);
        clientToServerPB.addToBuffer(data.cbegin(), data.cend());
        break;
      }
      case DllProtocol::CLIENT_SERVER_CONNECT: {
        TRACE_INFO("Client -> Server connect (%s)",
                   (dllPacket.getU8() == 0x00 ? "NOK" : "OK"));
        serverToClientPB.resetPosition();
        clientToServerPB.resetPosition();
        break;
      }
      case DllProtocol::CLIENT_SERVER_CLOSE_SOCKET: {
        TRACE_INFO("Client -> Server close socket (%s)",
                   (dllPacket.getU8() == 0x00 ? "NOK" : "OK"));
        break;
      }
      case DllProtocol::XTEA_KEY: {
        TRACE_INFO("XTEA Key");
        std::array<uint32_t, 4> xteaKey;
        for (int i = 0; i < 4; i++) {
          xteaKey[i] = dllPacket.getU32();
        }
        serverToClientPB.setXteaKey(xteaKey);
        clientToServerPB.setXteaKey(xteaKey);
        break;
      }
      default: {
        TRACE_ERROR("Unknown DLL packet type: 0x%x", packet_type);
        break;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  TRACE_INFO("Initalizing WSA");
  if (!WSAInit()) {
    TRACE_ERROR("Could not initialize WSA");
    return 1;
  }

  TRACE_INFO("Starting server");
  Server server = Server(8181);
  if (!server.setup()) {
    TRACE_ERROR("Could not start server");
  }

  TRACE_INFO("Finding Tibia pid");
  DWORD tibiaPid = findTibiaPid();
  if (tibiaPid == 0) {
    TRACE_ERROR("Could not find Tibia process");
    return 1;
  }

  TRACE_INFO("Injecting DLL");
  if (!injectDLL(tibiaPid)) {
    TRACE_ERROR("Could not inject DLL");
    return 1;
  }

  TRACE_INFO("Waiting for DLL to connect");
  SOCKET clientSocket = server.accept();
  if (clientSocket == INVALID_SOCKET) {
    TRACE_ERROR("Could not accept client connection");
    return 1;
  }

  TRACE_INFO("Loading modules");
  MarketData marketData;
  Map map;
  PacketDump packetDump;

  modules.push_back(&marketData);
  modules.push_back(&map);
  modules.push_back(&packetDump);

  TRACE_INFO("Starting recv loop");
  receiveLoop(clientSocket);

  return 0;
}
