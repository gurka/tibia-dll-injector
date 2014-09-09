#ifndef TIBIA_PROXY_DLL_H_
#define TIBIA_PROXY_DLL_H_

#define DLL_EXPORT __declspec(dllexport)

typedef int (WINAPI *PRECV)(SOCKET s, char* buf, int len, int flags);
typedef int (WINAPI *PSEND)(SOCKET s, char* buf, int len, int flags);
typedef int (WINAPI *PSOCKET)(int af, int type, int protocol);
typedef int (WINAPI *PHTONS)(u_short hostshort);
typedef int (WINAPI *PINET_ADDR)(const char* cp);
typedef int (WINAPI *PCONNECT)(SOCKET s, const struct sockaddr* name, int namelen);
typedef int (WINAPI *PCLOSESOCKET)(SOCKET s);

typedef int (WINAPI *PWSAGETLASTERROR)(void);
typedef void (WINAPI *PWSASETLASTERROR)(int);

int WINAPI myRecv(SOCKET s, char* buf, int len, int flags);
int WINAPI mySend(SOCKET s, char* buf, int len, int flags);
int WINAPI myConnect(SOCKET s, const struct sockaddr* name, int namelen);
int WINAPI myCloseSocket(SOCKET s);

void sendDllPacket(char* buf, int len);
void dllThreadFunc(HMODULE dllModule);

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

#endif  // TIBIA_PROXY_DLL_H_
