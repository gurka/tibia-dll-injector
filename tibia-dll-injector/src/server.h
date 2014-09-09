#ifndef SERVER_H_
#define SERVER_H_

#include <string>

class Server {
 public:
  Server(const std::string& port);
  virtual ~Server();

  bool setup();
  SOCKET accept();

 private:
  SOCKET socket_;
};

#endif  // SERVER_H_
