#include "../include/Client.hpp"
#include <unistd.h>

Client::Client(int fd)
    : _fd(fd), _hasNick(false), _hasUser(false), _isRegistered(false) ,_hostname("irc.server.local") {}

Client::~Client() { close(_fd); _hostname.clear(); }

int Client::getFd() const { return _fd; }

const std::string &Client::getNickname() const { return _nickname; }
void Client::setNickname(const std::string &nick) { _nickname = nick; }

const std::string &Client::getUsername() const { return _username; }
void Client::setUsername(const std::string &user) { _username = user; }

std::string &Client::getBuffer() { return _buffer; }

