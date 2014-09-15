#ifndef TIBIA_PROXY_DLL_H_
#define TIBIA_PROXY_DLL_H_

#define DLL_EXPORT __declspec(dllexport)

typedef int  (WINAPI *RECV_PTR)(SOCKET s, char* buf, int len, int flags);
typedef int  (WINAPI *SEND_PTR)(SOCKET s, char* buf, int len, int flags);
typedef int  (WINAPI *SOCKET_PTR)(int af, int type, int protocol);
typedef int  (WINAPI *HTONS_PTR)(u_short hostshort);
typedef int  (WINAPI *INET_ADDR_PTR)(const char* cp);
typedef int  (WINAPI *CONNECT_PTR)(SOCKET s, const struct sockaddr* name, int namelen);
typedef int  (WINAPI *CLOSESOCKET_PTR)(SOCKET s);
typedef int  (WINAPI *WSAGETLASTERROR_PTR)(void);
typedef void (WINAPI *WSASETLASTERROR_PTR)(int);

int WINAPI our_recv(SOCKET s, char* buf, int len, int flags);
int WINAPI our_send(SOCKET s, char* buf, int len, int flags);
int WINAPI our_connect(SOCKET s, const struct sockaddr* name, int namelen);
int WINAPI our_closesocket(SOCKET s);

void sendDllPacket();
void dllThreadFunc(HMODULE dllModule);

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

#endif  // TIBIA_PROXY_DLL_H_
