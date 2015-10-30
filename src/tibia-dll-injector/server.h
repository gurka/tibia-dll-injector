#ifndef SERVER_H_
#define SERVER_H_

#include <cstdint>

class Server {
 public:
  Server(uint16_t port);
  virtual ~Server();

  bool setup();
  SOCKET accept();

 private:
  const uint16_t port_;
  SOCKET socket_;
};

#endif  // SERVER_H_
