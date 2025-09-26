#pragma once

#include <iostream>
#include <set>
#include <sys/socket.h>

class Client
{
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
  std::string _hostname;
  std::set<std::string> _joinedChannels;

  // ayoub

  std::string file_buffer;

public:
  Client(int fd);
  ~Client();

  int getFd() const;
  const std::string &getNickname() const;
  void setNickname(const std::string &nick);

  const std::string &getUsername() const;
  void setUsername(const std::string &user);
  std::string &getBuffer();
  void sethostname(const char *s) { _hostname = s; };


  void setPassword(const std::string &pass) { _password = pass; }
  void setRealname(const std::string &realname) { _realname = realname; }
  void setHasNick(bool val) { _hasNick = val; }
  void setHasUser(bool val) { _hasUser = val; }
  void setRegistered(bool val) { _isRegistered = val; }
  void setHostname(const std::string &hostname) { _hostname = hostname; }

  const std::string &getPassword() const { return _password; }
  const std::string &getRealname() const { return _realname; }
  bool hasNick() const { return _hasNick; }
  bool hasUser() const { return _hasUser; }
  bool isRegistered() const { return _isRegistered; }

  bool isInChannel(const std::string &channel) const
  {
    if (_joinedChannels.find(channel) != _joinedChannels.end())
      return true;
    return false;
  }

  void joinChannel(std::string chanelName) { _joinedChannels.insert(chanelName); }
  void partChannel(std::string &channel) { _joinedChannels.erase(channel); }
  void clearBuffer() { _buffer.clear(); };


  // ayoub

  void sendMessage(const std::string &message)
  {
    send(getFd(), message.c_str(), message.size(), 0);
  }
    
    // Add file transfer buffer
    std::string& getFileBuffer() { return file_buffer; }
    void setFileBuffer(const std::string &buf) { file_buffer = buf; }

    const std::string &getHostname() const { return _hostname; }
};
