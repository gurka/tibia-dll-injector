#include <cstdio>
#include <cstdint>
#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <Winsock2.h>

#include "server.h"
#include "packet.h"
#include "xtea.h"
#include "bits.h"
#include "protocol.h"

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

  std::array<uint8_t, 1024 * 16> dllBuffer;

  std::array<uint8_t, 1024 * 16> serverToClientBuffer;
  uint16_t serverToClientPos = 0;

  std::array<uint8_t, 1024 * 16> clientToServerBuffer;
  uint16_t clientToServerPos = 0;

  uint32_t xteaKey[4];

  while (true) {
    // Get DLL packet length
    if (!recvAll(clientSocket, (char*)dllBuffer.begin(), 2)) {
      std::cerr << "Connection to DLL closed" << std::endl;
      break;
    }

    uint16_t dll_packet_length = Bits::getU16(dllBuffer, 0);

    // Get DLL packet
    if (!recvAll(clientSocket, (char*)dllBuffer.begin(), dll_packet_length)) {
      std::cerr << "Connection to DLL closed" << std::endl;
      break;
    }

    uint8_t packet_type = dllBuffer[0];
    switch (packet_type) {
    case 0x00:
      std::cout << "Server -> Client closed" << std::endl;
      break;
    case 0x01:
      std::cout << "Server -> Client error" << std::endl;
      break;
    case 0x02:
      //std::cout << "Server -> Client data" << std::endl;
      break;
    case 0x03:
      std::cout << "Client -> Server error" << std::endl;
      break;
    case 0x04:
      //std::cout << "Client -> Server data" << std::endl;
      break;
    case 0x05:
      std::cout << "Client -> Server connect ("
                << (dllBuffer[1] == 0x00 ? "NOK" : "OK") << ")" << std::endl;
      break;
    case 0x06:
      std::cout << "Client -> Server close socket ("
                << (dllBuffer[1] == 0x00 ? "NOK" : "OK") << ")" << std::endl;
      break;
    case 0x07:
      std::cout << "XTEA Key" << std::endl;
      break;
    default:
      std::cerr << "Unknown DLL packet type: " << (uint16_t)packet_type << std::endl;
      break;
    }

    if (packet_type == 0x02) {
      // Server -> Client data
      std::copy(&dllBuffer[1],
                &dllBuffer[dll_packet_length],
                &serverToClientBuffer[serverToClientPos]);
      serverToClientPos += (dll_packet_length - 1);

      // Check if we have complete packets
      while (serverToClientPos >= 2) {
        uint16_t packet_length = Bits::getU16(serverToClientBuffer, 0);
        if (2 + packet_length > serverToClientPos) {
          break;  // No more complete packets
        }

        //uint32_t checksum = Bits::toU32(serverToClientBuffer, 2);
        if (!Xtea::decrypt(xteaKey, &serverToClientBuffer[6], packet_length - 4)) {
          std::cerr << "Server -> Client Could not decrypt packet" << std::endl;
        } else {
          uint16_t decrypted_length = Bits::getU32(serverToClientBuffer, 6);
          if (decrypted_length > packet_length) {
            std::cerr << "Server -> Client Decrypted length (" << decrypted_length << ") > encrypted length (" << (packet_length - 4) << ")" << std::endl;
          } else {
            Packet packet(serverToClientBuffer.cbegin() + 8,
                          serverToClientBuffer.cbegin() + 8 + decrypted_length);
            uint8_t opcode = packet.getU8();
            std::map<uint8_t, std::string>::iterator opcodeStringIt = Protocol::gameServerOpcodes.find(opcode);
            if (opcodeStringIt != Protocol::gameServerOpcodes.end()) {
              std::cout << "Server -> Client [" << opcodeStringIt->second << "]" << std::endl;
            } else {
              std::cout << "Server -> Client <" << (uint16_t)opcode << ">" << std::endl;
            }
            /*
            std::cout << "Server -> Client Packet" << std::endl
                      << "\tType: " << (uint16_t)packet.getU8() << std::endl
                      << "\tEncrypted Length: " << packet_length << std::endl
                      << "\tDecrypted Length: " << decrypted_length << std::endl
                      << "\tChecksum: " << checksum << std::endl;
            */
          }
        }

        // Move rest of buffer (if any)
        uint16_t bytesLeft = serverToClientPos - (2 + packet_length);
        if (bytesLeft > 0) {
          std::copy(serverToClientBuffer.cbegin() + 2 + packet_length,
                    serverToClientBuffer.cbegin() + 2 + packet_length + bytesLeft,
                    serverToClientBuffer.begin());
        }
        serverToClientPos = bytesLeft;
      }
    } else if (packet_type == 0x04) {
      // Client -> Server data
      std::copy(&dllBuffer[1],
                &dllBuffer[dll_packet_length],
                &clientToServerBuffer[clientToServerPos]);
      clientToServerPos += (dll_packet_length - 1);

      // Check if we have a complete packet
      while (clientToServerPos >= 2) {
        uint16_t packet_length = Bits::getU16(clientToServerBuffer, 0);
        if (2 + packet_length > clientToServerPos) {
          break;  // No more complete packets
        }

        //uint32_t checksum = Bits::toU32(clientToServerBuffer, 2);
        if (!Xtea::decrypt(xteaKey, &clientToServerBuffer[6], packet_length - 4)) {
          std::cerr << "Client -> Server Could not decrypt packet" << std::endl;
        } else {
          uint16_t decrypted_length = Bits::getU16(clientToServerBuffer, 6);
          if (decrypted_length > packet_length) {
            std::cerr << "Client -> Server Decrypted length (" << decrypted_length << ") > encrypted length (" << (packet_length - 4) << ")" << std::endl;
          } else {
            Packet packet(clientToServerBuffer.cbegin() + 8,
                          clientToServerBuffer.cbegin() + 8 + decrypted_length);
            uint8_t opcode = packet.getU8();
            std::map<uint8_t, std::string>::iterator opcodeStringIt = Protocol::clientOpcodes.find(opcode);
            if (opcodeStringIt != Protocol::gameServerOpcodes.end()) {
              std::cout << "Client -> Server [" << opcodeStringIt->second << "]" << std::endl;
            } else {
              std::cout << "Client -> Server <" << (uint16_t)opcode << ">" << std::endl;
            }
            /*
            std::cout << "Client -> Server Packet" << std::endl
                      << "\tType: " << (uint16_t)packet.getU8() << std::endl
                      << "\tEncrypted Length: " << packet_length << std::endl
                      << "\tDecrypted Length: " << decrypted_length << std::endl
                      << "\tChecksum: " << checksum << std::endl;
            */
          }
        }

        // Move rest of buffer (if any)
        uint16_t bytesLeft = clientToServerPos - (2 + packet_length);
        if (bytesLeft > 0) {
          std::copy(clientToServerBuffer.cbegin() + 2 + packet_length,
                    clientToServerBuffer.cbegin() + 2 + packet_length + bytesLeft,
                    clientToServerBuffer.begin());
        }
        clientToServerPos = bytesLeft;
      }
    } else if (packet_type == 0x05) {
      // Client -> Server connect
      serverToClientPos = 0;
      clientToServerPos = 0;
    } else if (packet_type == 0x07) {
      // XTEA Key
      std::cout << std::hex;
      for (int i = 0; i < 4; i++) {
        xteaKey[i] = Bits::getU32(dllBuffer, 1 + (i * 4));
        std::cout << "0x" << xteaKey[i] << std::endl;
      }
      std::cout << std::dec;
    }
  }

  return 0;
}
