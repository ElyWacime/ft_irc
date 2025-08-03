#include <string>
#include <vector>

#include "../include/Client.hpp"
#include "../include/LoopDeLoop.hpp"
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

LoopDeLoop::LoopDeLoop(SocketZilla &_socket, std::string password,
                       SockItToMe &epoll_instance)
    : _serverSocket(_socket), _password(password), _poller(epoll_instance) {
  _poller.addFd(_serverSocket.getFd(), NULL);
}

LoopDeLoop::~LoopDeLoop() {
  for (std::map<int, Client *>::iterator it = _clients.begin();
       it != _clients.end(); ++it)
    delete it->second;
}
std::vector<std::string> LoopDeLoop::extractLines(std::string &buffer) {
  std::vector<std::string> lines;
  size_t pos;
  while ((pos = buffer.find("\r\n")) != std::string::npos) {
    lines.push_back(buffer.substr(0, pos));
    buffer.erase(0, pos + 2);
  }
  return lines;
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
          std::vector<std::string> lines = extractLines(client->getBuffer());
          for (size_t j = 0; j < lines.size(); ++j) {
            std::cout << "Received line: " << lines[j] << std::endl;
            // TODO: handle command parsing
          }
        }
      }
    }
  }
}
