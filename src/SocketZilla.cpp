#include "../include/SocketZilla.hpp"
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

SocketZilla::SocketZilla(int port)
{
  _sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (_sockfd < 0)
    throw std::runtime_error("Socket creation failed");

  int opt = 1;
  setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  memset(&_addr, 0, sizeof(_addr));
  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr = INADDR_ANY;
  _addr.sin_port = htons(port);

  if (bind(_sockfd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
    throw std::runtime_error("Bind failed");

  fcntl(_sockfd, F_SETFL, O_NONBLOCK);

  if (listen(_sockfd, 128) < 0)
    throw std::runtime_error("Listen failed");
}

SocketZilla::~SocketZilla() { close(_sockfd); }

int SocketZilla::getFd() const { return _sockfd; }
