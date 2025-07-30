#pragma once

#include "SockItToMe.hpp"
#include "SocketZilla.hpp"
#include <map>
#include <string>

class Client; // Forward declaration

class LoopDeLoop {
private:
  SocketZilla _serverSocket;
  SockItToMe _poller;
  std::map<int, Client *> _clients;

public:
  LoopDeLoop(int port);
  ~LoopDeLoop();

  void run();
};
