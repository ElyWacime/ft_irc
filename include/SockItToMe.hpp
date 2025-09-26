#pragma once
#include <sys/epoll.h>
#include <map>

// #include <sys/event.h>   // for kqueue
#include <sys/time.h>
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
