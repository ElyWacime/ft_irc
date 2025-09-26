#pragma once

#include "Channel.hpp"
#include "Client.hpp"
#include "SockItToMe.hpp"
#include "SocketZilla.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>

class Client;

class LoopDeLoop {
private:

  Client *_botClient;
  bool _botEnabled;
  std::vector<std::string> _badWords;
  void createBotClient();
  void addBotToChannel(const std::string& channelName);
  bool containsBadWords(const std::string& message);
  void handleBot(Client* sender, const std::string& channelName, const std::string& message);
    
  SocketZilla _serverSocket;
  std::string _password;
  SockItToMe _poller;
  std::map<int, Client *> _clients;
  std::map<std::string, Channel *> _channels;

  std::vector<std::string> extractLines(std::string &buffer);

  
  std::map<std::string, std::string> transfer_buffers_; 
  std::map<std::string, std::string> transfer_filenames_;
  std::map<std::string, bool> transfer_accepted_; 
    

public:

  LoopDeLoop(SocketZilla &_socket, std::string password,
             SockItToMe &epoll_instance);
  ~LoopDeLoop();
  void handleCommand(Client *client, const std::string &line);
  void run();
  void get_client_data(Client *client);

  bool nickExist(const std::string &nick) const {
    for (std::map<int, Client *>::const_iterator nit = _clients.begin();
         nit != _clients.end(); ++nit) {
      Client *client = nit->second;
      if (client->getNickname() == nick)
        return true;
    }
    return false;
  }

  
  std::string generateTransferKey(const std::string &from, const std::string &to);
  std::string generateServerFilename(const std::string &from, const std::string &to, const std::string &original_filename);
  void cleanupTransfer(const std::string &key);
  void handleFileTransferCommand(Client *client, const std::vector<std::string> &token);
  void  sendToNick(const std::string &nickname, const std::string  &message);
  Client* findClientByNick(const std::string &nickname);
  void handleCtcpMessage(Client *client, const std::string &target, const std::string &ctcpData);
};
