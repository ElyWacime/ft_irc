#include "../include/SockItToMe.hpp"
#include <stdexcept>
#include <unistd.h>

SockItToMe::SockItToMe() {
  _epollFd = epoll_create1(0);
  if (_epollFd < 0)
    throw std::runtime_error("epoll_create1 failed");
}

SockItToMe::~SockItToMe() { close(_epollFd); }

void SockItToMe::addFd(int fd, void *data) {
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.ptr = data;
  _fdToData[fd] = data;
  if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) < 0)
    throw std::runtime_error("epoll_ctl ADD failed");
}

void SockItToMe::removeFd(int fd) {
  epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, nullptr);
  _fdToData.erase(fd);
}

std::vector<struct epoll_event> SockItToMe::wait() {
  std::vector<struct epoll_event> events(64);
  int nfds = epoll_wait(_epollFd, events.data(), events.size(), -1);
  if (nfds < 0)
    throw std::runtime_error("epoll_wait failed");
  events.resize(nfds);
  return events;
}
