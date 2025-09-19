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
#include <sstream>
#include <vector>

class Client;
// class Channel;

class LoopDeLoop {
private:
  SocketZilla _serverSocket;
  std::string _password;
  SockItToMe _poller;
  std::map<int, Client *> _clients;
  std::map<std::string, Channel *> _channels;

  std::vector<std::string> extractLines(std::string &buffer);

  // File transfer - ayoub
  std::map<std::string, std::string> transfer_buffers_; // k -> file data
  std::map<std::string, std::string> transfer_filenames_; // k -> original filename
  std::map<std::string, bool> transfer_accepted_; // k -> acceptance status

  // Private helper methods for command handling
  void handleXferCommand(Client *client, std::istringstream &iss);
  void handlePassCommand(Client *client, std::istringstream &iss);
  void handleNickCommand(Client *client, std::istringstream &iss);
  void handleUserCommand(Client *client, std::istringstream &iss);
  void handleJoinCommand(Client *client, std::istringstream &iss);
  void handlePrivmsgCommand(Client *client, std::istringstream &iss);
  void handleKickCommand(Client *client, std::istringstream &iss);
  void handleInviteCommand(Client *client, std::istringstream &iss);
  void handleTopicCommand(Client *client, std::istringstream &iss);
  void handleModeCommand(Client *client, std::istringstream &iss);
  void handleUnknownCommand(Client *client, const std::string &command);

  // JOIN command helpers
  void joinSingleChannel(Client *client, const std::string &channelName, std::istringstream &iss);
  Channel* getOrCreateChannel(const std::string &channelName, Client *client);
  bool canJoinChannel(Client *client, Channel *channel, const std::string &channelName, std::istringstream &iss);
  void sendJoinNotifications(Client *client, Channel *channel, const std::string &channelName);
  void sendNamesList(Client *client, Channel *channel, const std::string &channelName);

  // PRIVMSG command helpers
  void sendChannelMessage(Client *client, const std::string &target, const std::string &message);
  void sendPrivateMessage(Client *client, const std::string &target, const std::string &message);

  // MODE command helpers
  void processModeChanges(Client *client, Channel *channel, const std::string &channelName, 
                         const std::string &modes, std::istringstream &iss);
  bool handleOperatorMode(Client *client, Channel *channel, const std::string &targetNick, bool adding);
  void sendChannelModes(Client *client, Channel *channel, const std::string &channelName);

  // Registration helpers
  void validateRegistration(Client *client);
  void sendWelcomeMessages(Client *client);

  // General helper methods
  Channel* findChannel(const std::string &channelName);
  Client* findClientByNickname(const std::string &nickname);

public:
  LoopDeLoop(SocketZilla &_socket, std::string password,
             SockItToMe &epoll_instance);
  ~LoopDeLoop();
  
  // Main command handler (now refactored)
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

  // File transfer methods - ayoub
  std::string generateTransferKey(const std::string &from, const std::string &to);
  std::string generateServerFilename(const std::string &from, const std::string &to, const std::string &original_filename);
  void cleanupTransfer(const std::string &key);
  void handleFileTransferCommand(Client *client, const std::vector<std::string> &token);
  void sendToNick(const std::string &nickname, const std::string &message);
  Client* findClientByNick(const std::string &nickname);






  //ayoub
  void sendWelcomeMessages(Client *client)
  Channel* findChannel(const std::string &channelName)
  Client*:findClientByNickname(const std::string &nickname);
};