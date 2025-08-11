#pragma once

#include <iostream>

class Client
{
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
