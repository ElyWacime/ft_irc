#pragma once

#include "SockItToMe.hpp"
#include "SocketZilla.hpp"
#include <cstdlib>
#include <cstring> // for memset if needed
#include <fcntl.h> // for fcntl, F_SETFL, O_NONBLOCK
#include <iostream>
#include <map>
#include <string>
#include <unistd.h> // for close

class Client; // Forward declaration

class LoopDeLoop {
private:
  SocketZilla _serverSocket;
  std::string _password;
  SockItToMe _poller;
  std::map<int, Client *> _clients;

  std::vector<std::string> extractLines(std::string &buffer);

public:
  LoopDeLoop(SocketZilla &_socket, std::string password,
             SockItToMe &epoll_instance);
  ~LoopDeLoop();
  void handleCommand(Client *client, const std::string &line);
  void run();
  void get_client_data(Client *client);
};
