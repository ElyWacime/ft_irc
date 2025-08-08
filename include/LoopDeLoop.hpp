#pragma once

#include "SockItToMe.hpp"
#include "SocketZilla.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>

class Client;
class Channel;

class LoopDeLoop {
private:
  SocketZilla _serverSocket;
  std::string _password;
  SockItToMe _poller;
  std::map<int, Client *> _clients;
  std::map<std::string, Channel *> _channels;

  std::vector<std::string> extractLines(std::string &buffer);

public:
  LoopDeLoop(SocketZilla &_socket, std::string password,
             SockItToMe &epoll_instance);
  ~LoopDeLoop();
  void handleCommand(Client *client, const std::string &line);
  void run();
  void get_client_data(Client *client);
  bool nickExist(const std::string &nick) const {
    for (std::map<int, Client *>::const_iterator nit = _clients.begin();
         nit != _clients.end(); ++nit) {
      Client *client = nit->second;
      if (client->getNickname() == nick)
        return true;
    }
    return false;
  }
};
