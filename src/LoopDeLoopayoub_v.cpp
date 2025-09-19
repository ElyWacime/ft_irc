#include <string>
#include <vector>

#include "../include/LoopDeLoop.hpp"
#include "../include/bot.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>


std::string LoopDeLoop::generateTransferKey(const std::string &from, const std::string &to) {
    return from + "->" + to;
}

std::string LoopDeLoop::generateServerFilename(const std::string &from, const std::string &to, const std::string &original_filename) {
    return from + "to" + to + "_" + original_filename;
}

Client* LoopDeLoop::findClientByNick(const std::string &nickname) {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == nickname && it->second->isRegistered()) {
            return it->second;
        }
    }
    return NULL;
}

void LoopDeLoop::sendToNick(const std::string &nickname, const std::string  &message)
{
  Client *target = findClientByNick(nickname);
  target->sendMessage(message);
}

void LoopDeLoop::cleanupTransfer(const std::string &key)
{
  transfer_buffers_.erase(key);
  transfer_filenames_.erase(key);
  transfer_accepted_.erase(key);
}

void LoopDeLoop::handleFileTransferCommand(Client *client, const std::vector<std::string> &token)
{
  if (!client->isRegistered())
  {
    std::string err = ":server 451 <JOIN> :You have not registered";
    client->sendMessage(err);
    return;
  }
  std::string cmd = token[0];
  if (cmd == "OFFER" && token.size() >= 3)
  {
    std::string target = token[1];
    std::string filename = token[2];

    
    Client* targetClient = findClientByNick(target);
    if (!targetClient) {
        std::string err = "401 " + target + " :No such nick/channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    std::string key = generateTransferKey(client->getNickname(), target);
    cleanupTransfer(key);
    
    transfer_filenames_[key] = filename;
    transfer_accepted_[key] = false;
    transfer_buffers_[key] = "";
    
    sendToNick(target, ":" + client->getNickname() + " XFER REQUEST " + filename + "\r\n");
    client->sendMessage("File offer sent to " + target + "\r\n");
  }
  else if (cmd == "ACCEPT" && token.size() >= 3)
  {
    std::string sender = token[1];
    std::string filename = token[2];
    Client* senderClient = findClientByNick(sender);

    if (!senderClient)
    {
      std::string err = "401 " + sender + " :No such client\r\n";
      client->sendMessage(err);
      return;
    }
    
    std::string key = generateTransferKey(sender, client->getNickname());
    
    if (transfer_filenames_.count(key) > 0 && transfer_filenames_[key] == filename)
    {
      transfer_accepted_[key] = true;
      sendToNick(sender, ":" + client->getNickname() + " XFER ACCEPTED " + filename + "\r\n");
      client->sendMessage("NOTICE :Ready to receive file sender " + sender + "\r\n");
    }
    else
    {
      client->sendMessage("NOTICE :No pending file transfer sender " + sender + "\r\n");
    }
  }
  else if (cmd == "SENDDATA" && token.size() == 3)
  {
      std::string target = token[1];
      std::string filename = token[2];
      
      Client* targetClient = findClientByNick(target);
      if (!targetClient) {
          std::string err = "401 " + target + " :No such nick/channel\r\n";
          send(client->getFd(), err.c_str(), err.size(), 0);
          return;
      }
      
      std::string key = generateTransferKey(client->getNickname(), target);
      
      if (transfer_accepted_.count(key) > 0 && transfer_accepted_[key] && 
          transfer_filenames_[key] == filename) {
          
          // Read source file in chunks
          std::ifstream source_file(filename.c_str(), std::ios::binary);
          if (!source_file) {
              client->sendMessage("NOTICE :Cannot open file: " + filename + "\r\n");
              return;
          }
          
          // Generate server filename
          std::string server_filename = generateServerFilename(client->getNickname(), target, filename);
          std::ofstream dest_file(server_filename.c_str(), std::ios::binary);
          
          if (!dest_file) {
              client->sendMessage("NOTICE :Cannot create server file\r\n");
              source_file.close();
              return;
          }
          
          // Read and write in chunks internally (no messages to user)
          const size_t CHUNK_SIZE = 8192; // 8KB chunks
          char buffer[CHUNK_SIZE];
          size_t total_bytes = 0;
          
          while (source_file.read(buffer, CHUNK_SIZE) || source_file.gcount() > 0) {
              size_t bytes_read = source_file.gcount();
              dest_file.write(buffer, bytes_read);
              total_bytes += bytes_read;
              
              // Chunking happens internally, no messages sent to user
          }
          
          source_file.close();
          dest_file.close();
          
          // Notify recipient that file is ready
          Client* targetClient = findClientByNick(target);
          if (targetClient) {
              sendToNick(target, ":" + client->getNickname() + " XFER READY " + server_filename + "\r\n");
              {
                  std::ostringstream oss;
                  oss << "NOTICE :File transfer completed (" << total_bytes << " bytes)\r\n";
                  client->sendMessage(oss.str());
              }
          } else {
              client->sendMessage("NOTICE :Target client not found for file transfer\r\n");
          }
          
          // Cleanup
          cleanupTransfer(key);
      }
  }
  else if (cmd == "DOWNLOAD" && token.size() == 3)
  {
      std::string server_filename = token[1];
      
      std::ifstream source_file(server_filename.c_str(), std::ios::binary);
      
      if (source_file) {
          // Extract original filename from server filename
          size_t underscore_pos = server_filename.find('_');
          if (underscore_pos == std::string::npos) {
              client->sendMessage("NOTICE :Invalid server filename\r\n");
              return;
          }
          
          std::string original_filename = "new" + server_filename.substr(underscore_pos + 1);
          
          // Create the file for receiver
          std::ofstream dest_file(original_filename.c_str(), std::ios::binary);
          
          if (!dest_file) {
              client->sendMessage("NOTICE :Cannot create file: " + original_filename + "\r\n");
              source_file.close();
              return;
          }
          
          // Read and write in chunks internally
          const size_t CHUNK_SIZE = 8192;
          char buffer[CHUNK_SIZE];
          size_t total_bytes = 0;
          
          while (source_file.read(buffer, CHUNK_SIZE) || source_file.gcount() > 0) {
              size_t bytes_read = source_file.gcount();
              dest_file.write(buffer, bytes_read);
              total_bytes += bytes_read;
              
              // Chunking happens internally, no messages sent to user
          }
          
          source_file.close();
          dest_file.close();
          
          {
              std::ostringstream oss;
              oss << "NOTICE :File downloaded as: " << original_filename 
                  << " (" << total_bytes << " bytes)\r\n";
              client->sendMessage(oss.str());
          }
          
            std::ostringstream oss;
            oss << "NOTICE :File downloaded as: " << original_filename 
              << " (" << total_bytes << " bytes)\r\n";
            client->sendMessage(oss.str());
      } else {
          client->sendMessage("NOTICE :File not found: " + server_filename + "\r\n");
      }


}
}



LoopDeLoop::LoopDeLoop(SocketZilla &_socket, std::string password,
                       SockItToMe &epoll_instance)
    : _serverSocket(_socket), _password(password), _poller(epoll_instance) {
  _poller.addFd(_serverSocket.getFd(), NULL);
}

LoopDeLoop::~LoopDeLoop() {
  for (std::map<int, Client *>::iterator it = _clients.begin();
       it != _clients.end(); ++it)
    delete it->second;
}
std::vector<std::string> LoopDeLoop::extractLines(std::string &buffer) {
  std::vector<std::string> lines;
  size_t pos;
  while ((pos = buffer.find("\r\n")) != std::string::npos) {
    lines.push_back(buffer.substr(0, pos));
    buffer.erase(0, pos + 2);
  }
  return lines;
}







// Main command handler - now much cleaner
void LoopDeLoop::handleCommand(Client *client, const std::string &line)
{
    std::istringstream iss(line);
    std::string command;
    iss >> command;

    // Handle bot commands (starting with /)
    if (!command.empty() && command[0] == '/') {
        bot bot;
        bot.bot_handle(client, line);
        return;
    }

    // Route to specific command handlers
    if (command == "XFER") {
        handleXferCommand(client, iss);
    } else if (command == "PASS") {
        handlePassCommand(client, iss);
    } else if (command == "NICK") {
        handleNickCommand(client, iss);
    } else if (command == "USER") {
        handleUserCommand(client, iss);
    } else if (command == "JOIN") {
        handleJoinCommand(client, iss);
    } else if (command == "PRIVMSG") {
        handlePrivmsgCommand(client, iss);
    } else if (command == "KICK") {
        handleKickCommand(client, iss);
    } else if (command == "INVITE") {
        handleInviteCommand(client, iss);
    } else if (command == "TOPIC") {
        handleTopicCommand(client, iss);
    } else if (command == "MODE") {
        handleModeCommand(client, iss);
    } else {
        handleUnknownCommand(client, command);
    }

    // Check for registration completion
    validateRegistration(client);
}

// File transfer command handler
void LoopDeLoop::handleXferCommand(Client *client, std::istringstream &iss)
{
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 3) {
        handleFileTransferCommand(client, tokens);
    }
}

// Password command handler
void LoopDeLoop::handlePassCommand(Client *client, std::istringstream &iss)
{
    std::string pass;
    iss >> pass;
    client->setPassword(pass);
}

// Nickname command handler - HexChat compatible
void LoopDeLoop::handleNickCommand(Client *client, std::istringstream &iss)
{
    std::string nick;
    iss >> nick;
    
    if (nickExist(nick)) {
        // HexChat expects proper IRC error format
        std::string err = ":server 433 * " + nick + " :Nickname is already in use\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }
    
    client->setNickname(nick);
    client->setHasNick(true);
    
    // Send nick confirmation for HexChat
    if (client->isRegistered()) {
        std::string nickMsg = ":" + client->getNickname() + " NICK :" + nick + "\r\n";
        send(client->getFd(), nickMsg.c_str(), nickMsg.size(), 0);
    }
}

// User registration command handler - HexChat compatible
void LoopDeLoop::handleUserCommand(Client *client, std::istringstream &iss)
{
    std::string username, unused, unused2, realname;
    iss >> username >> unused >> unused2;
    std::getline(iss, realname);
    
    if (!realname.empty() && realname[0] == ':') {
        realname.erase(0, 1);
    }
    
    client->setUsername(username);
    client->setRealname(realname);
    client->setHasUser(true);
}

// Join command handler - HexChat compatible
void LoopDeLoop::handleJoinCommand(Client *client, std::istringstream &iss)
{
    if (!client->isRegistered()) {
        std::string err = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string channelList;
    iss >> channelList;

    if (channelList.empty()) {
        std::string err = ":server 461 " + client->getNickname() + " JOIN :Not enough parameters\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::stringstream ss(channelList);
    std::string channelName;
    
    while (std::getline(ss, channelName, ',')) {
        joinSingleChannel(client, channelName, iss);
    }
}

// Helper function for joining a single channel
void LoopDeLoop::joinSingleChannel(Client *client, const std::string &channelName, std::istringstream &iss)
{
    if (channelName.empty() || channelName[0] != '#') {
        std::string err = ":server 476 " + client->getNickname() + " " + channelName + " :Bad Channel Mask\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    if (client->isInChannel(channelName)) {
        std::string err = ":server 443 " + client->getNickname() + " " + channelName + " :is already on channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    Channel *channel = getOrCreateChannel(channelName, client);
    
    // Check channel restrictions
    if (!canJoinChannel(client, channel, channelName, iss)) {
        return;
    }

    // Join the channel
    channel->addClient(client);
    client->joinChannel(channelName);

    // Send join notifications and channel info (HexChat compatible)
    sendJoinNotifications(client, channel, channelName);
}

// Get or create channel
Channel* LoopDeLoop::getOrCreateChannel(const std::string &channelName, Client *client)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
    
    if (it == _channels.end()) {
        Channel *channel = new Channel(channelName);
        channel->addOperator(client); // First user becomes operator
        _channels[channelName] = channel;
        return channel;
    }
    
    return it->second;
}

// Check if client can join channel
bool LoopDeLoop::canJoinChannel(Client *client, Channel *channel, const std::string &channelName, std::istringstream &iss)
{
    // Check invite-only mode
    if (channel->isInviteOnly() && !channel->isInvited(client)) {
        std::string err = ":server 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return false;
    }

    // Check channel key
    if (channel->hasKey()) {
        std::string key;
        iss >> key;
        if (key.empty() || key != channel->getKey()) {
            std::string err = ":server 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n";
            send(client->getFd(), err.c_str(), err.size(), 0);
            return false;
        }
    }

    // Check user limit
    if (channel->getUserLimit() > 0 && channel->getClientCount() >= channel->getUserLimit()) {
        std::string err = ":server 471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return false;
    }

    return true;
}

// Send join notifications - HexChat compatible
void LoopDeLoop::sendJoinNotifications(Client *client, Channel *channel, const std::string &channelName)
{
    // Send JOIN message to all channel members
    std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + " JOIN :" + channelName + "\r\n";
    channel->broadcast(joinMsg, NULL);
    
    // Send channel topic if it exists
    if (!channel->getTopic().empty()) {
        std::string topicMsg = ":server 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
        send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
    } else {
        // Send "no topic" message for HexChat
        std::string noTopicMsg = ":server 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
        send(client->getFd(), noTopicMsg.c_str(), noTopicMsg.size(), 0);
    }
    
    // Send NAMES list (HexChat expects this)
    sendNamesList(client, channel, channelName);
}

// Send names list - HexChat compatible
void LoopDeLoop::sendNamesList(Client *client, Channel *channel, const std::string &channelName)
{
    std::string namesList = ":server 353 " + client->getNickname() + " = " + channelName + " :";
    
    // Add all channel members with proper prefixes
    const std::vector<Client*>& clients = channel->getClients();
    for (size_t i = 0; i < clients.size(); ++i) {
        if (channel->isOperator(clients[i])) {
            namesList += "@";
        }
        namesList += clients[i]->getNickname();
        if (i < clients.size() - 1) {
            namesList += " ";
        }
    }
    
    namesList += "\r\n";
    send(client->getFd(), namesList.c_str(), namesList.size(), 0);
    
    std::string endNames = ":server 366 " + client->getNickname() + " " + channelName + " :End of NAMES list\r\n";
    send(client->getFd(), endNames.c_str(), endNames.size(), 0);
}

// Private message handler - HexChat compatible
void LoopDeLoop::handlePrivmsgCommand(Client *client, std::istringstream &iss)
{
    if (!client->isRegistered()) {
        std::string err = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string target;
    iss >> target;

    std::string message;
    std::getline(iss, message);
    if (!message.empty() && message[0] == ':') {
        message.erase(0, 1);
    }

    if (target.empty() || message.empty()) {
        std::string err = ":server 461 " + client->getNickname() + " PRIVMSG :Not enough parameters\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    if (target[0] == '#') {
        sendChannelMessage(client, target, message);
    } else {
        sendPrivateMessage(client, target, message);
    }
}

// Send message to channel
void LoopDeLoop::sendChannelMessage(Client *client, const std::string &target, const std::string &message)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(target);
    if (it == _channels.end()) {
        std::string err = ":server 403 " + client->getNickname() + " " + target + " :No such channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    Channel *channel = it->second;

    if (!client->isInChannel(target)) {
        std::string err = ":server 442 " + client->getNickname() + " " + target + " :You're not on that channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    // HexChat compatible format with full hostmask
    std::string fullMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                         " PRIVMSG " + target + " :" + message + "\r\n";
    channel->broadcast(fullMsg, client);
}

// Send private message to user
void LoopDeLoop::sendPrivateMessage(Client *client, const std::string &target, const std::string &message)
{
    Client *targetClient = findClientByNickname(target);
    if (!targetClient) {
        std::string err = ":server 401 " + client->getNickname() + " " + target + " :No such nick/channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string fullMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                         " PRIVMSG " + target + " :" + message + "\r\n";
    send(targetClient->getFd(), fullMsg.c_str(), fullMsg.size(), 0);
}

// Kick command handler
void LoopDeLoop::handleKickCommand(Client *client, std::istringstream &iss)
{
    if (!client->isRegistered()) {
        std::string err = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string channelName, targetNick, reason;
    iss >> channelName >> targetNick;
    std::getline(iss, reason);
    
    if (!reason.empty() && reason[0] == ':') {
        reason.erase(0, 1);
    }
    if (reason.empty()) {
        reason = targetNick; // Default reason
    }

    Channel *channel = findChannel(channelName);
    if (!channel) return;

    if (!channel->isOperator(client)) {
        std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    Client *target = findClientByNickname(targetNick);
    if (!target || !channel->hasClient(target)) {
        std::string err = ":server 441 " + client->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    // HexChat compatible kick message
    std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                         " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg, NULL);
    
    channel->removeClient(target);
    target->partChannel(channelName);
}

// Invite command handler
void LoopDeLoop::handleInviteCommand(Client *client, std::istringstream &iss)
{
    if (!client->isRegistered()) {
        std::string err = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string targetNick, channelName;
    iss >> targetNick >> channelName;

    Channel *channel = findChannel(channelName);
    if (!channel) {
        std::string err = ":server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    if (!channel->isOperator(client)) {
        std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    Client *target = findClientByNickname(targetNick);
    if (!target) {
        std::string err = ":server 401 " + client->getNickname() + " " + targetNick + " :No such nick/channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    channel->addInvited(target);

    // Send invite to target
    std::string inviteMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                           " INVITE " + targetNick + " " + channelName + "\r\n";
    send(target->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);

    // Send confirmation to inviter (HexChat expects this)
    std::string confirmMsg = ":server 341 " + client->getNickname() + " " + targetNick + " " + channelName + "\r\n";
    send(client->getFd(), confirmMsg.c_str(), confirmMsg.size(), 0);
}

// Topic command handler
void LoopDeLoop::handleTopicCommand(Client *client, std::istringstream &iss)
{
    if (!client->isRegistered()) {
        std::string err = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string channelName;
    iss >> channelName;

    Channel *channel = findChannel(channelName);
    if (!channel) {
        std::string err = ":server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string newTopic;
    std::getline(iss, newTopic);
    if (!newTopic.empty() && newTopic[0] == ':') {
        newTopic.erase(0, 1);
    }

    if (newTopic.empty()) {
        // Show current topic
        if (channel->getTopic().empty()) {
            std::string msg = ":server 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
            send(client->getFd(), msg.c_str(), msg.size(), 0);
        } else {
            std::string msg = ":server 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
            send(client->getFd(), msg.c_str(), msg.size(), 0);
        }
    } else {
        // Set new topic
        if (channel->isTopicRestricted() && !channel->isOperator(client)) {
            std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
            send(client->getFd(), err.c_str(), err.size(), 0);
            return;
        }

        channel->setTopic(newTopic);
        std::string topicMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                              " TOPIC " + channelName + " :" + newTopic + "\r\n";
        channel->broadcast(topicMsg, NULL);
    }
}

// Mode command handler
void LoopDeLoop::handleModeCommand(Client *client, std::istringstream &iss)
{
    if (!client->isRegistered()) {
        std::string err = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    std::string channelName, modes;
    iss >> channelName >> modes;

    Channel *channel = findChannel(channelName);
    if (!channel) {
        std::string err = ":server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    if (modes.empty()) {
        // Send current channel modes
        sendChannelModes(client, channel, channelName);
        return;
    }

    if (!channel->isOperator(client)) {
        std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    processModeChanges(client, channel, channelName, modes, iss);
}

// Process mode changes
void LoopDeLoop::processModeChanges(Client *client, Channel *channel, const std::string &channelName, 
                                   const std::string &modes, std::istringstream &iss)
{
    bool adding = true;
    std::string modeChanges = "";
    std::string modeParams = "";
    
    for (size_t i = 0; i < modes.size(); ++i) {
        char m = modes[i];
        
        if (m == '+') {
            adding = true;
        } else if (m == '-') {
            adding = false;
        } else {
            std::string param;
            bool needsParam = false;
            
            switch (m) {
                case 'i':
                    channel->setInviteOnly(adding);
                    modeChanges += m;
                    break;
                case 't':
                    channel->setTopicRestricted(adding);
                    modeChanges += m;
                    break;
                case 'k':
                    if (adding) {
                        iss >> param;
                        if (!param.empty()) {
                            channel->setKey(param);
                            channel->setHasKey(true);
                            needsParam = true;
                        }
                    } else {
                        channel->setKey("");
                        channel->setHasKey(false);
                    }
                    modeChanges += m;
                    break;
                case 'l':
                    if (adding) {
                        iss >> param;
                        int limit = std::atoi(param.c_str());
                        if (limit > 0) {
                            channel->setUserLimit(limit);
                            needsParam = true;
                        }
                    } else {
                        channel->setUserLimit(-1);
                    }
                    modeChanges += m;
                    break;
                case 'o':
                    iss >> param;
                    if (handleOperatorMode(client, channel, param, adding)) {
                        modeChanges += m;
                        needsParam = true;
                    }
                    break;
                default:
                    std::string err = ":server 472 " + client->getNickname() + " " + std::string(1, m) + " :is unknown mode char to me\r\n";
                    send(client->getFd(), err.c_str(), err.size(), 0);
                    continue;
            }
            
            if (needsParam && !param.empty()) {
                if (!modeParams.empty()) modeParams += " ";
                modeParams += param;
            }
        }
    }
    
    // Send mode change notification if any modes were changed
    if (!modeChanges.empty()) {
        std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + 
                             " MODE " + channelName + " " + (adding ? "+" : "-") + modeChanges;
        if (!modeParams.empty()) {
            modeMsg += " " + modeParams;
        }
        modeMsg += "\r\n";
        channel->broadcast(modeMsg, NULL);
    }
}

// Handle operator mode changes
bool LoopDeLoop::handleOperatorMode(Client *client, Channel *channel, const std::string &targetNick, bool adding)
{
    Client *target = findClientByNickname(targetNick);
    if (!target || !channel->hasClient(target)) {
        std::string err = ":server 441 " + client->getNickname() + " " + targetNick + " " + channel->getName() + " :They aren't on that channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return false;
    }
    
    if (adding) {
        channel->addOperator(target);
    } else {
        channel->removeOperator(target);
    }
    
    return true;
}

// Send current channel modes
void LoopDeLoop::sendChannelModes(Client *client, Channel *channel, const std::string &channelName)
{
    std::string modes = "+";
    std::string params = "";
    
    if (channel->isInviteOnly()) modes += "i";
    if (channel->isTopicRestricted()) modes += "t";
    if (channel->hasKey()) {
        modes += "k";
        params += " " + channel->getKey();
    }
    if (channel->getUserLimit() > 0) {
        modes += "l";
        params += " " + std::to_string(channel->getUserLimit());
    }
    
    std::string modeMsg = ":server 324 " + client->getNickname() + " " + channelName + " " + modes + params + "\r\n";
    send(client->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
}

// Handle unknown commands
void LoopDeLoop::handleUnknownCommand(Client *client, const std::string &command)
{
    std::string response = ":server 421 " + client->getNickname() + " " + command + " :Unknown command\r\n";
    send(client->getFd(), response.c_str(), response.size(), 0);
}

// Validate client registration
void LoopDeLoop::validateRegistration(Client *client)
{
    if (!client->isRegistered() && client->hasNick() && client->hasUser()) {
        if (client->getPassword() != _password) {
            std::string msg = ":server 464 " + client->getNickname() + " :Password incorrect\r\n";
            send(client->getFd(), msg.c_str(), msg.length(), 0);
            _poller.removeFd(client->getFd());
            close(client->getFd());
            _clients.erase(client->getFd());
            delete client;
            return;
        }

        client->setRegistered(true);
        
        // Send welcome messages (HexChat expects these)
        sendWelcomeMessages(client);
    }
}

// Send welcome messages - HexChat compatible
void LoopDeLoop::sendWelcomeMessages(Client *client)
{
    std::string nick = client->getNickname();
    
    // RPL_WELCOME (001)
    std::string welcome = ":server 001 " + nick + " :Welcome to the IRC Network " + nick + "!" + 
                         client->getUsername() + "@" + client->getHostname() + "\r\n";
    send(client->getFd(), welcome.c_str(), welcome.length(), 0);
    
    // RPL_YOURHOST (002)
    std::string yourhost = ":server 002 " + nick + " :Your host is server, running version 1.0\r\n";
    send(client->getFd(), yourhost.c_str(), yourhost.length(), 0);
    
    // RPL_CREATED (003)
    std::string created = ":server 003 " + nick + " :This server was created today\r\n";
    send(client->getFd(), created.c_str(), created.length(), 0);
    
    // RPL_MYINFO (004)
    std::string myinfo = ":server 004 " + nick + " server 1.0 o iklto\r\n";
    send(client->getFd(), myinfo.c_str(), myinfo.length(), 0);
}

// Helper functions
Channel* LoopDeLoop::findChannel(const std::string &channelName)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
    return (it != _channels.end()) ? it->second : NULL;
}

Client* LoopDeLoop::findClientByNickname(const std::string &nickname)
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == nickname) {
            return it->second;
        }
    }
    return NULL;
}








void LoopDeLoop::run() {
  while (true) {
    std::vector<struct epoll_event> events = _poller.wait();
    for (size_t i = 0; i < events.size(); ++i) {
      if (events[i].data.ptr == NULL) {
        // New connection
        int clientFd = accept(_serverSocket.getFd(), NULL, NULL);
        if (clientFd < 0)
          continue;
        fcntl(clientFd, F_SETFL, O_NONBLOCK);
        Client *client = new Client(clientFd);
        _clients[clientFd] = client;
        _poller.addFd(clientFd, client);
        std::cout << "New client accepted: " << clientFd << std::endl;
      } else {
        // Existing client
        Client *client = static_cast<Client *>(events[i].data.ptr);
        char buf[512];
        int n = recv(client->getFd(), buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
          std::cout << "Client disconnected: " << client->getFd() << std::endl;
          _poller.removeFd(client->getFd());
          close(client->getFd());
          delete client;
          _clients.erase(client->getFd());
        } else {
          buf[n] = '\0';
          client->getBuffer().append(buf);
          std::vector<std::string> lines = extractLines(client->getBuffer());
          for (size_t j = 0; j < lines.size(); ++j) {
            handleCommand(client, lines[i]);
          }
        }
      }
    }
  }
}
