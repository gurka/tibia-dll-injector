#include <winsock2.h>
#include <Ws2tcpip.h>
#include "server.h"

Server::Server(uint16_t port)
  : port_(port),
    socket_(INVALID_SOCKET) {
}

Server::~Server() {
  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
  }
}

bool Server::setup() {
  socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_ == INVALID_SOCKET) {
    return false;
  }

  sockaddr_in service;
  service.sin_family = AF_INET;
  service.sin_addr.s_addr = inet_addr("127.0.0.1");
  service.sin_port = htons(port_);
  if (bind(socket_, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    return false;
  }
  if (listen(socket_, 8) == SOCKET_ERROR) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    return false;
  }

  return true;
}

SOCKET Server::accept() {
  return ::accept(socket_, NULL, NULL);
}
