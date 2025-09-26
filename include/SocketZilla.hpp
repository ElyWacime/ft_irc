#pragma once
#include <cerrno>
#include <netinet/in.h>
#include <string>

class SocketZilla {
private:
  int _sockfd;
  struct sockaddr_in _addr;

public:
  SocketZilla(int port);
  ~SocketZilla();

  int getFd() const;
  void setup();
};
