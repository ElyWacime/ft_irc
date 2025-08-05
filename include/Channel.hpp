#include "Client.hpp"
#include <set>
#include <sys/socket.h>
#include <sys/types.h>

class Channel {
private:
  std::string _name;
  std::set<Client *> _clients; // or std::vector if you prefer

public:
  Channel(const std::string &name) : _name(name) {}
  const std::string &getName() const { return _name; }

  void addClient(Client *client) { _clients.insert(client); }
  void removeClient(Client *client) { _clients.erase(client); }
  bool hasClient(Client *client) const {
    return _clients.find(client) != _clients.end();
  }

  void broadcast(const std::string &msg, Client *sender = nullptr) {
    for (Client *c : _clients) {
      if (c != sender) {
        send(c->getFd(), msg.c_str(), msg.size(), 0);
      }
    }
  }
};
