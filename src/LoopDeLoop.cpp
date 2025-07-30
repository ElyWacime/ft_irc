#include <string>
#include <vector>

#include "../include/Client.hpp"
#include "../include/LoopDeLoop.hpp"
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

LoopDeLoop::LoopDeLoop(int port, std::string password)
    : _serverSocket(port), _password(password) {
  _poller.addFd(_serverSocket.getFd(), NULL);
}

LoopDeLoop::~LoopDeLoop() {
  for (std::map<int, Client *>::iterator it = _clients.begin();
       it != _clients.end(); ++it)
    delete it->second;
}

void LoopDeLoop::run() {
  while (true) {
    std::vector<struct epoll_event> events = _poller.wait();
    for (size_t i = 0; i < events.size(); ++i) {
      if (events[i].data.ptr == NULL) {
        // New connection
        int clientFd = accept(_serverSocket.getFd(), NULL, NULL);
        if (clientFd < 0)
          continue;
        fcntl(clientFd, F_SETFL, O_NONBLOCK);
        Client *client = new Client(clientFd);
        _clients[clientFd] = client;
        _poller.addFd(clientFd, client);
        std::cout << "New client accepted: " << clientFd << std::endl;
      } else {
        // Existing client
        Client *client = static_cast<Client *>(events[i].data.ptr);
        char buf[512];
        int n = recv(client->getFd(), buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
          std::cout << "Client disconnected: " << client->getFd() << std::endl;
          _poller.removeFd(client->getFd());
          close(client->getFd());
          delete client;
          _clients.erase(client->getFd());
        } else {
          buf[n] = '\0';
          client->getBuffer().append(buf);
          // TODO: handle command parsing
        }
      }
    }
  }
}
