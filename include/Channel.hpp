#include "Client.hpp"
#include <set>
#include <sys/socket.h>
#include <sys/types.h>

class Channel {
private:
  std::string _name;
  std::set<Client *> _clients;

public:
  Channel(const std::string &name) : _name(name) {}
  const std::string &getName() const { return _name; }

  void addClient(Client *client) { _clients.insert(client); }
  void removeClient(Client *client) { _clients.erase(client); }
  bool hasClient(Client *client) const {
    return _clients.find(client) != _clients.end();
  }

  void broadcast(const std::string &msg, Client *sender = NULL) {
    for (std::set<Client *>::iterator it = _clients.begin();
         it != _clients.end(); it++) {
      Client *c = *it;
      if (c != sender) {
        send(c->getFd(), msg.c_str(), msg.size(), 0);
      }
    }
  }
};
