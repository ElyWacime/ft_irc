#include <string>
#include <vector>

#include "../include/LoopDeLoop.hpp"
#include "../include/bot.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

void trim(std::string &str) {
  size_t start = 0;
  size_t end = str.length();

  while (start < end && isspace(str[start])) {
    ++start;
  }

  while (start < end && str[start] != ':') {
    ++start;
  }

  if (start < end && str[start] == ':') {
    ++start;
  }

  str = str.substr(start, end - start);
}
std::string LoopDeLoop::generateTransferKey(const std::string &from, const std::string &to) {
  return from + "->" + to;
}

std::string
LoopDeLoop::generateServerFilename(const std::string &from, const std::string &to,
  const std::string &original_filename) {
  return from + "to" + to + "_" + original_filename;
}

Client *LoopDeLoop::findClientByNick(const std::string &nickname) {
  for (std::map<int, Client *>::iterator it = _clients.begin();
       it != _clients.end(); ++it) {
    if (it->second->getNickname() == nickname && it->second->isRegistered()) {
      return it->second;
    }
  }
  return NULL;
}

void LoopDeLoop::sendToNick(const std::string &nickname,
                            const std::string &message) {
  Client *target = findClientByNick(nickname);
  target->sendMessage(message);
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

void LoopDeLoop::createBotClient() {
  _botClient = new Client(-1);
  _botClient->setNickname("daveBot");
  _botClient->setUsername("davebot");
  _botClient->setRealname("Moderation Bot");
  _botClient->setHostname("server");
  _botClient->setRegistered(true);
  _botEnabled = true;

  _badWords.push_back("fuck");
  _badWords.push_back("badword");
  _badWords.push_back("idiot");
  _badWords.push_back("stupid");
  _badWords.push_back("hate");
}

void LoopDeLoop::addBotToChannel(const std::string &channelName) {
  if (!_botEnabled || !_botClient)
    return;
  std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
  if (it == _channels.end())
    return;

  Channel *channel = it->second;
  if (channel->hasClient(_botClient))
    return;

  channel->addClient(_botClient);
  _botClient->joinChannel(channelName);

  std::string joinMsg =
      ":" + _botClient->getNickname() + "!" + _botClient->getUsername() + "@" +
      _botClient->getHostname() + " JOIN :" + channelName + "\r\n";

  const std::vector<Client *> &clients = channel->getClients();
  for (size_t i = 0; i < clients.size(); ++i) {
    Client *client = clients[i];
    if (client->getFd() != -1) {
      send(client->getFd(), joinMsg.c_str(), joinMsg.size(), 0);
    }
  }

  std::string welcomeMsg =
      ":" + _botClient->getNickname() + " PRIVMSG " + channelName +
      " :🤖 ModBot has joined to help moderate this channel!\r\n";
  for (size_t i = 0; i < clients.size(); ++i) {
    Client *client = clients[i];
    if (client->getFd() != -1) {
      send(client->getFd(), welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
  }

}

bool LoopDeLoop::containsBadWords(const std::string &message) {
  std::string lowerMessage = message;
  std::transform(lowerMessage.begin(), lowerMessage.end(), lowerMessage.begin(),
                 ::tolower);

  for (size_t i = 0; i < _badWords.size(); ++i) {
    if (lowerMessage.find(_badWords[i]) != std::string::npos) {
      return true;
    }
  }
  return false;
}
void LoopDeLoop::handleBot(Client *sender, const std::string &channelName,
                           const std::string &message) {
  if (!_botEnabled || !_botClient)
    return;

  std::istringstream iss(message);
  std::string command, args;
  iss >> command >> args;
  if (!command.empty() && command[0] == ':')
    command.erase(0, 1);
  if (command.empty())
    return;
  if (command[0] == '@' || command.length() > 1) {
    if (command == "@help" && args.empty()) {
      std::string helpMsg =
          ":" + _botClient->getNickname() + " PRIVMSG " + channelName +
          " :Available commands: joke, quote, ascii, coin, dice\r\n";
      sendToNick(sender->getNickname(), helpMsg);
      return;
    } else if (command == "@joke" && args.empty()) {
      bot b;
      std::string joke = b.get_random_joke();
      std::string jokeMsg = ":" + _botClient->getNickname() + " PRIVMSG " +
                            channelName + " :" + joke + "\r\n";
      sendToNick(sender->getNickname(), jokeMsg);
      return;
    } else if (command == "@quote" && args.empty()) {
      bot b;
      std::string quote = b.get_random_quote();
      std::string quoteMsg = ":" + _botClient->getNickname() + " PRIVMSG " +
                             channelName + " :" + quote + "\r\n";
      sendToNick(sender->getNickname(), quoteMsg);
      return;
    }
     else if (command == "@coin" && args.empty()) {
      bot b;
      std::string coin = b.get_random_coin();
      std::string coinMsg = ":" + _botClient->getNickname() + " PRIVMSG " +
                            channelName + " :" + coin + "\r\n";
      sendToNick(sender->getNickname(), coinMsg);
      return;
    } else if (command == "@dice" && args.empty()) {
      bot b;
      std::string dice = b.get_random_dice();
      std::string diceMsg = ":" + _botClient->getNickname() + " PRIVMSG " +
                            channelName + " :" + sender->getNickname() +
                            " rolled a " + dice + "\r\n";
      sendToNick(sender->getNickname(), diceMsg);
      return;
    }
  }

  if (containsBadWords(message)) {
    std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
    if (it == _channels.end())
      return;
    Channel *channel = it->second;

    std::string warningMsg = ":" + _botClient->getNickname() + " PRIVMSG " +
                             channelName + " :" + sender->getNickname() +
                             ", please watch your language!\r\n";

    const std::vector<Client *> &clients = channel->getClients();
    for (size_t i = 0; i < clients.size(); ++i) {
      Client *client = clients[i];
      if (client->getFd() != -1) {
        send(client->getFd(), warningMsg.c_str(), warningMsg.size(), 0);
      }
    }

  }
}

void LoopDeLoop::handleCommand(Client *client, const std::string &line) {
  std::istringstream iss(line);

  bot bot;
  std::string command;
  iss >> command;

  if (command == "PASS") {
    std::string pass;
    iss >> pass;

    if (!pass.empty() && pass[pass.size() - 1] == '\n')
      pass.erase(pass.size() - 1);
    if (!pass.empty() && pass[pass.size() - 1] == '\r')
      pass.erase(pass.size() - 1);

    if (pass != _password) {

      std::string err = ":server 464 * :Password incorrect\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      _poller.removeFd(client->getFd());
      close(client->getFd());
      _clients.erase(client->getFd());
      delete client;
      return;
    }
    client->setPassword(pass);
  } else if (command == "NICK") {
    std::string nick;
    iss >> nick;

    if (!nick.empty() && nick[nick.size() - 1] == '\n')
      nick.erase(nick.size() - 1);
    if (!nick.empty() && nick[nick.size() - 1] == '\r')
      nick.erase(nick.size() - 1);

    if (nickExist(nick)) {
      std::string err =
          ":server 433 * " + nick + " :Nickname is already in use\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }
    client->setNickname(nick);
    client->setHasNick(true);
  } else if (command == "USER") {
    std::string username, unused, unused2, realname;
    iss >> username >> unused >> unused2;
    std::getline(iss, realname);
    if (!realname.empty() && realname[0] == ':')
      realname.erase(0, 1);
    if (!realname.empty() && realname[realname.size() - 1] == '\n')
      realname.erase(realname.size() - 1);
    if (!realname.empty() && realname[realname.size() - 1] == '\r')
      realname.erase(realname.size() - 1);

    client->setUsername(username);
    client->setRealname(realname);
    client->setHasUser(true);
  } else if (command == "JOIN") {
    if (!client->isRegistered()) {
      std::string err = ":server 451 " + client->getNickname() +
                        " JOIN :You have not registered\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }
    std::string channelList;
    iss >> channelList;

    if (channelList.empty()) {
      std::string err = ":server 461 " + client->getNickname() +
                        " JOIN :Not enough parameters\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }

    std::stringstream ss(channelList);
    std::string channelName;
    while (std::getline(ss, channelName, ',')) {
      if (channelName.empty() || channelName[0] != '#') {
        std::string err = ":server 476 " + client->getNickname() + " " +
                          channelName + " :Bad Channel Mask\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        continue;
      }

      if (client->isInChannel(channelName)) {
        std::string err = ":server 443 " + client->getNickname() + " " +
                          channelName + " :is already on channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        continue;
      }

      Channel *channel = NULL;
      std::map<std::string, Channel *>::iterator it =
          _channels.find(channelName);
      if (it == _channels.end()) {
        channel = new Channel(channelName);
        channel->addOperator(client);
        _channels[channelName] = channel;

        addBotToChannel(channelName);
        channel->addOperator(_botClient);
       
      } else {
        channel = it->second;
      }

      if (channel->isInviteOnly() && (!channel->isInvated(client))) {
        std::string err = ":server 473 " + client->getNickname() + " " +
                          channelName + " :Cannot join channel (+i)\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        continue;
      }

      if (channel->hasKey()) {
        std::string key;
        iss >> key;
        if (key.empty() || key != channel->getKey()) {
          std::string err = ":server 475 " + client->getNickname() + " " +
                            channelName + " :Cannot join channel (+k)\r\n";
          send(client->getFd(), err.c_str(), err.size(), 0);
          continue;
        }
      }

      channel->addClient(client);
      client->joinChannel(channelName);

      std::string joinMsg = ":" + client->getNickname() + "!" +
                            client->getUsername() +
                            "@"+client->getHostname()+" JOIN :" + channelName + "\r\n";
      channel->broadcast(joinMsg, NULL);

      if (!channel->getTopic().empty()) {
        std::string topicMsg = ":server 332 " + client->getNickname() + " " +
                               channelName + " :" + channel->getTopic() +
                               "\r\n";
        send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
      } else {
        std::string noTopicMsg = ":server 331 " + client->getNickname() + " " +
                                 channelName + " :No topic is set\r\n";
        send(client->getFd(), noTopicMsg.c_str(), noTopicMsg.size(), 0);
      }

      std::string namesList =
          ":server 353 " + client->getNickname() + " = " + channelName + " :";
      const std::vector<Client *> &members = channel->getClients();
      for (size_t i = 0; i < members.size(); ++i) {
        if (channel->isOperator(members[i])) {
          namesList += "@"; 
        }
        namesList += members[i]->getNickname();
        if (i < members.size() - 1) {
          namesList += " ";
        }
      }
      namesList += "\r\n";
      send(client->getFd(), namesList.c_str(), namesList.size(), 0);

      std::string endNames = ":server 366 " + client->getNickname() + " " +
                             channelName + " :End of NAMES list\r\n";
      send(client->getFd(), endNames.c_str(), endNames.size(), 0);

      std::string modeMsg = ":server 324 " + client->getNickname() + " " +
                            channelName + " +t\r\n";
      send(client->getFd(), modeMsg.c_str(), modeMsg.size(), 0);

      for (size_t i = 0; i < members.size(); ++i) {
        std::string whoMsg =
            ":server 352 " + client->getNickname() + " " + channelName + " " +
            members[i]->getNickname() + " " + members[i]->getUsername() +
            " localhost localhost " + members[i]->getNickname() +
            " H@ :" + members[i]->getRealname() + "\r\n";
        send(client->getFd(), whoMsg.c_str(), whoMsg.size(), 0);
      }
      std::string endWho = ":server 315 " + client->getNickname() + " " +
                           channelName + " :End of WHO list\r\n";
      send(client->getFd(), endWho.c_str(), endWho.size(), 0);
    }
  } else if (command == "PRIVMSG") {
    if (!client->isRegistered()) {
      std::string err = ":server 451 " + client->getNickname() +
                        " :You have not registered\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }

    std::string target;
    iss >> target;

    std::string message;
    std::getline(iss, message);
    trim(message);

    if (!message.empty() && message[0] == ':')
      message.erase(0, 1);

    if (target.empty() || message.empty()) {
      std::string err = ":server 461 " + client->getNickname() +
                        " PRIVMSG :Not enough parameters\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }

    if (!message.empty() && message.find("DCC ") != std::string::npos) {
      std::string ctcpData = message.substr(1, message.length() - 2);
      handleCtcpMessage(client, target, ctcpData);
      return;
    }
    if (target[0] == '#') {
      std::map<std::string, Channel *>::iterator it = _channels.find(target);
      if (it == _channels.end()) {
        std::string err = ":server 403 " + client->getNickname() + " " +
                          target + " :No such channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
      }
      handleBot(client, target, message);

      Channel *channel = it->second;

      if (!client->isInChannel(target)) {
        std::string err = ":server 442 " + client->getNickname() + " " +
                          target + " :You're not on that channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
      }

      std::string fullMsg = ":" + client->getNickname() + "!" +
                            client->getUsername() + "@localhost" + " PRIVMSG " +
                            target + " :" + message + "\r\n";
      channel->broadcast(fullMsg, client);
    } else {
      Client *targetClient = findClientByNick(target);
      if (!targetClient) {
        std::string err = ":server 401 " + client->getNickname() + " " +
                          target + " :No such nick/channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
      }

      std::string fullMsg = ":" + client->getNickname() + "!" +
                            client->getUsername() + "@localhost" + " PRIVMSG " +
                            target + " :" + message + "\r\n";
      send(targetClient->getFd(), fullMsg.c_str(), fullMsg.size(), 0);
    }
  }

  else if (command == "KICK") {
    if (!client->isRegistered()) {
      std::string err = ":server 451 " + client->getNickname() +
                        " KICK :You have not registered\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }
    std::string channelName, targetNick, reason;
    iss >> channelName >> targetNick;
    std::getline(iss, reason);
    if (!reason.empty() && reason[0] == ':')
      reason.erase(0, 1);
    if (reason.empty())
      reason = targetNick;

    std::map<std::string, Channel *>::iterator chIt =
        _channels.find(channelName);
    if (chIt == _channels.end())
      return;

    Channel *channel = chIt->second;

    if (!channel->isOperator(client)) {
      std::string err = ":server 482 " + client->getNickname() + " " +
                        channelName + " :You're not channel operator\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }

    Client *target = NULL;
    for (std::map<int, Client *>::iterator cit = _clients.begin();
         cit != _clients.end(); ++cit) {
      if (cit->second->getNickname() == targetNick) {
        target = cit->second;
        break;
      }
    }
    if (!target || !channel->hasClient(target)) {
      std::string msg = "client doesn't exist in : " + channel->getName();
      send(client->getFd(), msg.c_str(), msg.size(), 0);
      return;
    }

    std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName +
                          " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg, NULL);
    channel->removeClient(target);
    target->partChannel(channelName);
  } else if (command == "INVITE") {
    if (!client->isRegistered()) {
      std::string err = ":server 451 " + client->getNickname() +
                        " INVITE :You have not registered\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }
    std::string targetNick, channelName;
    iss >> targetNick >> channelName;

    std::map<std::string, Channel *>::iterator chIt =
        _channels.find(channelName);
    if (chIt == _channels.end())
      return;

    Channel *channel = chIt->second;

    if (!channel->isOperator(client)) {
      std::string err = ":server 482 " + client->getNickname() + " " +
                        channelName + " :You're not channel operator\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }

    Client *target = NULL;
    for (std::map<int, Client *>::iterator cit = _clients.begin();
         cit != _clients.end(); ++cit) {
      if (cit->second->getNickname() == targetNick) {
        target = cit->second;
        break;
      }
    }
    if (!target)
      return;

    channel->addInvited(target);

    std::string inviteMsg = ":" + client->getNickname() + " INVITE " +
                            targetNick + " " + channelName + "\r\n";
    send(target->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);
  } else if (command == "TOPIC") {
    if (!client->isRegistered()) {
      std::string err = ":server 451 " + client->getNickname() +
                        " TOPIC :You have not registered\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }
    std::string channelName;
    iss >> channelName;

    std::map<std::string, Channel *>::iterator chIt =
        _channels.find(channelName);
    if (chIt == _channels.end())
      return;

    Channel *channel = chIt->second;

    std::string newTopic;
    std::getline(iss, newTopic);
    if (!newTopic.empty() && newTopic[0] == ':')
      newTopic.erase(0, 1);

    if (newTopic.empty()) {
      std::string topicMsg = ":server 332 " + client->getNickname() + " " +
                             channelName + " :" + channel->getTopic() + "\r\n";
      send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
    } else {
      if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        std::string err = ":server 482 " + client->getNickname() + " " +
                          channelName + " :You're not channel operator\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
      }
      channel->setTopic(newTopic);
      std::string topicMsg = ":" + client->getNickname() + " TOPIC " +
                             channelName + " :" + newTopic + "\r\n";
      channel->broadcast(topicMsg, NULL);
    }
  } else if (command == "MODE") {
    if (!client->isRegistered()) {
      std::string err = ":server 451 " + client->getNickname() +
                        " MODE :You have not registered\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }

    std::string channelName, modes, param;
    iss >> channelName >> modes >> param;

    std::map<std::string, Channel *>::iterator chIt =
        _channels.find(channelName);
    if (chIt == _channels.end())
      return;

    Channel *channel = chIt->second;

    if (!channel->isOperator(client)) {
      std::string err = ":server 482 " + client->getNickname() + " " +
                        channelName + " :You're not channel operator\r\n";
      send(client->getFd(), err.c_str(), err.size(), 0);
      return;
    }

    bool adding = true;
    for (size_t i = 0; i < modes.size(); ++i) {
      char m = modes[i];
      if (m == '+')
        adding = true;
      else if (m == '-')
        adding = false;
      else if (m == 'i')
        channel->setInviteOnly(adding);
      else if (m == 't')
        channel->setTopicRestricted(adding);
      else if (m == 'k') {
        if (adding) {
          if (param.empty())
            continue;
          channel->setHasKey(adding);
          channel->setKey(param);
        } else
          channel->setKey("");
      } else if (m == 'l') {
        if (adding) {
          try {
            int n = std::atoi(param.c_str());
            if (n < (int)channel->getClients().size()) {
              std::string errstring =
                  "421 " + client->getNickname() + " :Invalid user limit\r\n";
              send(client->getFd(), errstring.c_str(), errstring.size(), 0);
              continue;
            }
            channel->setUserLimit(n);
            std::string valstring = " Chanel Limit Changed Succesfully\r\n";
            send(client->getFd(), valstring.c_str(), valstring.size(), 0);
          } catch (const std::exception &e) {
            std::string errstring =
                "401 " + client->getNickname() + ":Invalid Number\r\n";
            send(client->getFd(), errstring.c_str(), errstring.size(), 0);
            continue;
          }
        } else
          channel->setUserLimit(-1);
      } else if (m == 'o') {
        Client *target = NULL;
        for (std::map<int, Client *>::iterator cit = _clients.begin();
             cit != _clients.end(); ++cit) {
          if (cit->second->getNickname() == param) {
            target = cit->second;
            break;
          }
        }
        if (target && channel->hasClient(target)) {
          if (adding)
            channel->addOperator(target);
          else
            channel->removeOperator(target);
        }
      } else {
        std::string err = ":server 472 " + client->getNickname() + " ";
        err += m;
        err += " :is unknown mode char to me\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
      }
    }
  } else {

  }

  if (!client->isRegistered() && client->hasNick() && client->hasUser()) {

    if (!_password.empty() && client->getPassword() != _password) {
      std::string msg =
          ":server 464 " + client->getNickname() + " :Password incorrect\r\n";
      send(client->getFd(), msg.c_str(), msg.length(), 0);
      _poller.removeFd(client->getFd());
      close(client->getFd());
      _clients.erase(client->getFd());
      delete client;
      return;
    }

    client->setRegistered(true);
    std::string welcome = ":server 001 " + client->getNickname() +
                          " :Welcome to the IRC server!\r\n";
    send(client->getFd(), welcome.c_str(), welcome.length(), 0);
  }
}

void LoopDeLoop::handleCtcpMessage(Client *client, const std::string &target,
                                   const std::string &ctcpData) {
  std::istringstream iss(ctcpData);
  std::string ctcpCommand;
  iss >> ctcpCommand;

  if (ctcpCommand.find("DCC") != std::string::npos) {
    std::string dccType;
    iss >> dccType;


    if (dccType == "SEND") {
      std::string filename, host, port, filesize;

      iss >> std::ws;

      if (iss.peek() == '"') {
        char quote;
        iss >> quote;
        std::getline(iss, filename, '"');
        filename = "\"" + filename + "\"";
      } else {
        iss >> filename;
      }

      iss >> host >> port >> filesize;

      Client *targetClient = findClientByNick(target);
      if (!targetClient) {
        std::string err = ":server 401 " + client->getNickname() + " " +
                          target + " :No such nick/channel\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
      }

      std::string dccMsg =
          ":" + client->getNickname() + "!" + client->getUsername() + "@" +
          client->getHostname() + " PRIVMSG " + target + " :\001DCC SEND " +
          filename + " " + host + " " + port + " " + filesize + "\001\r\n";

      send(targetClient->getFd(), dccMsg.c_str(), dccMsg.size(), 0);

      std::string confirmMsg = ":server NOTICE " + client->getNickname() +
                               " :DCC SEND request forwarded to " + target +
                               "\r\n";
      send(client->getFd(), confirmMsg.c_str(), confirmMsg.size(), 0);
    }
  }
}

void LoopDeLoop::run() {

  createBotClient();
  while (true) {
    std::vector<struct epoll_event> events = _poller.wait();
    for (size_t i = 0; i < events.size(); ++i) {
      if (events[i].data.ptr == NULL) {
        // New connection
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientFd =
            accept(_serverSocket.getFd(),
                   reinterpret_cast<struct sockaddr *>(&clientAddr), &addrLen);
        getpeername(clientFd, (struct sockaddr *)&clientAddr, &addrLen);
        char ipStr[INET_ADDRSTRLEN];
                               
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
        if (clientFd < 0)
          continue;
        fcntl(clientFd, F_SETFL, O_NONBLOCK);
        Client *client = new Client(clientFd);
        _clients[clientFd] = client;
        _clients[clientFd]->setHostname(ipStr);
        _poller.addFd(clientFd, client);
        std::cout << "New client accepted: " << clientFd << std::endl;
      } else {
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

