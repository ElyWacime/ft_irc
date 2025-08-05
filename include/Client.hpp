#pragma once

#include <iostream>
#include <set>

class Client {
private:
  int _fd;
  std::string _buffer;
  std::string _nickname;
  std::string _username;
  std::string _realname;
  std::string _password;
  bool _hasNick;
  bool _hasUser;
  bool _isRegistered;
  std::set<std::string> _joinedChannels;

public:
  Client(int fd);
  ~Client();

  int getFd() const;
  const std::string &getNickname() const;
  void setNickname(const std::string &nick);
  const std::string &getUsername() const;
  void setUsername(const std::string &user);
  std::string &getBuffer();
  void setPassword(const std::string &pass) { _password = pass; }
  const std::string &getPassword() const { return _password; }

  void setRealname(const std::string &realname) { _realname = realname; }
  const std::string &getRealname() const { return _realname; }

  void setHasNick(bool val) { _hasNick = val; }
  void setHasUser(bool val) { _hasUser = val; }
  bool hasNick() const { return _hasNick; }
  bool hasUser() const { return _hasUser; }
  bool isRegistered() const { return _isRegistered; }
  void setRegistered(bool val) { _isRegistered = val; }

  void joinChannel(std::string chanelName) {
    _joinedChannels.insert(chanelName);
  }

  void clearBuffer() { _buffer.clear(); };
};
