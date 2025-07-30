#include <string>
#include <vector>

class Client {
private:
  int _fd;
  std::string _nickname;
  std::string _username;
  std::string _buffer;

public:
  Client(int fd);
  ~Client();

  int getFd() const;
  const std::string &getNickname() const;
  void setNickname(const std::string &nick);
  const std::string &getUsername() const;
  void setUsername(const std::string &user);
  std::string &getBuffer();
};

#endif

// Client.cpp
#include "Client.hpp"
#include <unistd.h>

Client::Client(int fd) : _fd(fd) {}

Client::~Client() { close(_fd); }

int Client::getFd() const { return _fd; }

const std::string &Client::getNickname() const { return _nickname; }
void Client::setNickname(const std::string &nick) { _nickname = nick; }

const std::string &Client::getUsername() const { return _username; }
void Client::setUsername(const std::string &user) { _username = user; }

std::string &Client::getBuffer() { return _buffer; }

// LoopDeLoop.cpp (skeleton)
#include "Client.hpp"
#include "LoopDeLoop.hpp"
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

LoopDeLoop::LoopDeLoop(int port) : _serverSocket(port) {
  _poller.addFd(_serverSocket.getFd(), nullptr);
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
      if (events[i].data.ptr == nullptr) {
        // New connection
        int clientFd = accept(_serverSocket.getFd(), nullptr, nullptr);
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
