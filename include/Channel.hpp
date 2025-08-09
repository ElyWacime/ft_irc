#include "Client.hpp"
#include <set>
#include <sys/socket.h>
#include <sys/types.h>

class Channel {
private:
  std::string _name;
  std::string _topic;
  std::string _key;
  int _userLimit;
  bool _inviteOnly;
  bool _topicRestricted;
  bool _hasKey;

  std::set<Client *> _clients;
  std::set<Client *> _operators;
  std::set<Client *> _invited;

public:
  Channel(const std::string &name)
      : _name(name), _userLimit(-1), _inviteOnly(0), _topicRestricted(0),
        _hasKey(0) {}
  const std::string &getName() const { return _name; }
  const std::string &getTopic() const { return _topic; }
  void setTopic(const std::string &topic) { _topic = topic; }

  void addClient(Client *client) { _clients.insert(client); }
  void removeClient(Client *client) {
    _clients.erase(client);
    _operators.erase(client);
    _invited.erase(client);
  }
  bool hasClient(Client *client) const {
    return _clients.find(client) != _clients.end();
  }
  const std::set<Client *> &getClients() const { return _clients; }
  
  bool nickExist(const std::string &nick) const {
    for (std::set<Client *>::const_iterator nit = _clients.begin();
         nit != _clients.end(); nit++) {
      if ((*nit)->getNickname() == nick)
        return true;
    }
    return false;
  }
  bool userExist(const std::string &nick) const {
    for (std::set<Client *>::const_iterator uit = _clients.begin();
         uit != _clients.end(); uit++) {
      if ((*uit)->getNickname() == nick)
        return true;
    }
    return false;
  }

  void addOperator(Client *client) { _operators.insert(client); }
  void removeOperator(Client *client) { _operators.erase(client); }
  bool isOperator(Client *client) {
    return _operators.find(client) != _operators.end();
  }

  void addInvited(Client *client) { _invited.insert(client); }
  bool isInvated(Client *client) const {
    return _invited.find(client) != _invited.end();
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

  void setInviteOnly(bool flag) { _inviteOnly = flag; }
  bool isInviteOnly() const { return _inviteOnly; }

  void setTopicRestricted(bool flag) { _topicRestricted = flag; }
  bool isTopicRestricted() const { return _topicRestricted; }

  void setKey(const std::string &key) { _key = key; }
  const std::string &getKey() const { return _key; }
  void setHasKey(bool flag) { _hasKey = flag; }
  bool hasKey() const { return _hasKey; }

  void setUserLimit(int userLimit) { _userLimit = userLimit; }
  int getUserLimit() const { return _userLimit; }

  size_t clientCount() const { return _clients.size(); }
};
