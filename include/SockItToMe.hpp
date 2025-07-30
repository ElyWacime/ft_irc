#pragma once

#include <map>
#include <sys/epoll.h>
#include <vector>

class SockItToMe {
private:
  int _epollFd;
  std::map<int, void *> _fdToData;

public:
  SockItToMe();
  ~SockItToMe();

  void addFd(int fd, void *data);
  void removeFd(int fd);
  std::vector<struct epoll_event> wait();
};
