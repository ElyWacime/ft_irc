// #include <string>
// #include <vector>

// #include "../include/LoopDeLoop.hpp"
// #include "../include/bot.hpp"
// #include <cstring>
// #include <iostream>
// #include <sstream>
// #include <sys/socket.h>
// #include <fcntl.h>
// #include <fstream>
// #include <unistd.h>


// std::string LoopDeLoop::generateTransferKey(const std::string &from, const std::string &to) {
//     return from + "->" + to;
// }

// std::string LoopDeLoop::generateServerFilename(const std::string &from, const std::string &to, const std::string &original_filename) {
//     return from + "to" + to + "_" + original_filename;
// }

// Client* LoopDeLoop::findClientByNick(const std::string &nickname) {
//     for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
//         if (it->second->getNickname() == nickname && it->second->isRegistered()) {
//             return it->second;
//         }
//     }
//     return NULL;
// }

// void LoopDeLoop::sendToNick(const std::string &nickname, const std::string  &message)
// {
//   Client *target = findClientByNick(nickname);
//   target->sendMessage(message);
// }

// void LoopDeLoop::cleanupTransfer(const std::string &key)
// {
//   transfer_buffers_.erase(key);
//   transfer_filenames_.erase(key);
//   transfer_accepted_.erase(key);
// }

// void LoopDeLoop::handleFileTransferCommand(Client *client, const std::vector<std::string> &token)
// {
//   if (!client->isRegistered())
//   {
//     std::string err = ":server 451 <JOIN> :You have not registered";
//     client->sendMessage(err);
//     return;
//   }
//   std::string cmd = token[0];
//   if (cmd == "OFFER" && token.size() >= 3)
//   {
//     std::string target = token[1];
//     std::string filename = token[2];

    
//     Client* targetClient = findClientByNick(target);
//     if (!targetClient) {
//         std::string err = "401 " + target + " :No such nick/channel\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//         return;
//     }
    
//     std::string key = generateTransferKey(client->getNickname(), target);
//     cleanupTransfer(key);
    
//     transfer_filenames_[key] = filename;
//     transfer_accepted_[key] = false;
//     transfer_buffers_[key] = "";
    
//     sendToNick(target, ":" + client->getNickname() + " XFER REQUEST " + filename + "\r\n");
//     client->sendMessage("File offer sent to " + target + "\r\n");
//   }
//   else if (cmd == "ACCEPT" && token.size() >= 3)
//   {
//     std::string sender = token[1];
//     std::string filename = token[2];
//     Client* senderClient = findClientByNick(sender);

//     if (!senderClient)
//     {
//       std::string err = "401 " + sender + " :No such client\r\n";
//       client->sendMessage(err);
//       return;
//     }
    
//     std::string key = generateTransferKey(sender, client->getNickname());
    
//     if (transfer_filenames_.count(key) > 0 && transfer_filenames_[key] == filename)
//     {
//       transfer_accepted_[key] = true;
//       sendToNick(sender, ":" + client->getNickname() + " XFER ACCEPTED " + filename + "\r\n");
//       client->sendMessage("NOTICE :Ready to receive file sender " + sender + "\r\n");
//     }
//     else
//     {
//       client->sendMessage("NOTICE :No pending file transfer sender " + sender + "\r\n");
//     }
//   }
//   else if (cmd == "SENDDATA" && token.size() == 3)
//   {
//       std::string target = token[1];
//       std::string filename = token[2];
      
//       Client* targetClient = findClientByNick(target);
//       if (!targetClient) {
//           std::string err = "401 " + target + " :No such nick/channel\r\n";
//           send(client->getFd(), err.c_str(), err.size(), 0);
//           return;
//       }
      
//       std::string key = generateTransferKey(client->getNickname(), target);
      
//       if (transfer_accepted_.count(key) > 0 && transfer_accepted_[key] && 
//           transfer_filenames_[key] == filename) {
          
//           // Read source file in chunks
//           std::ifstream source_file(filename.c_str(), std::ios::binary);
//           if (!source_file) {
//               client->sendMessage("NOTICE :Cannot open file: " + filename + "\r\n");
//               return;
//           }
          
//           // Generate server filename
//           std::string server_filename = generateServerFilename(client->getNickname(), target, filename);
//           std::ofstream dest_file(server_filename.c_str(), std::ios::binary);
          
//           if (!dest_file) {
//               client->sendMessage("NOTICE :Cannot create server file\r\n");
//               source_file.close();
//               return;
//           }
          
//           // Read and write in chunks internally (no messages to user)
//           const size_t CHUNK_SIZE = 8192; // 8KB chunks
//           char buffer[CHUNK_SIZE];
//           size_t total_bytes = 0;
          
//           while (source_file.read(buffer, CHUNK_SIZE) || source_file.gcount() > 0) {
//               size_t bytes_read = source_file.gcount();
//               dest_file.write(buffer, bytes_read);
//               total_bytes += bytes_read;
              
//               // Chunking happens internally, no messages sent to user
//           }
          
//           source_file.close();
//           dest_file.close();
          
//           // Notify recipient that file is ready
//           Client* targetClient = findClientByNick(target);
//           if (targetClient) {
//               sendToNick(target, ":" + client->getNickname() + " XFER READY " + server_filename + "\r\n");
//               {
//                   std::ostringstream oss;
//                   oss << "NOTICE :File transfer completed (" << total_bytes << " bytes)\r\n";
//                   client->sendMessage(oss.str());
//               }
//           } else {
//               client->sendMessage("NOTICE :Target client not found for file transfer\r\n");
//           }
          
//           // Cleanup
//           cleanupTransfer(key);
//       }
//   }
//   else if (cmd == "DOWNLOAD" && token.size() == 3)
//   {
//       std::string server_filename = token[1];
      
//       std::ifstream source_file(server_filename.c_str(), std::ios::binary);
      
//       if (source_file) {
//           // Extract original filename from server filename
//           size_t underscore_pos = server_filename.find('_');
//           if (underscore_pos == std::string::npos) {
//               client->sendMessage("NOTICE :Invalid server filename\r\n");
//               return;
//           }
          
//           std::string original_filename = "new" + server_filename.substr(underscore_pos + 1);
          
//           // Create the file for receiver
//           std::ofstream dest_file(original_filename.c_str(), std::ios::binary);
          
//           if (!dest_file) {
//               client->sendMessage("NOTICE :Cannot create file: " + original_filename + "\r\n");
//               source_file.close();
//               return;
//           }
          
//           // Read and write in chunks internally
//           const size_t CHUNK_SIZE = 8192;
//           char buffer[CHUNK_SIZE];
//           size_t total_bytes = 0;
          
//           while (source_file.read(buffer, CHUNK_SIZE) || source_file.gcount() > 0) {
//               size_t bytes_read = source_file.gcount();
//               dest_file.write(buffer, bytes_read);
//               total_bytes += bytes_read;
              
//               // Chunking happens internally, no messages sent to user
//           }
          
//           source_file.close();
//           dest_file.close();
          
//           {
//               std::ostringstream oss;
//               oss << "NOTICE :File downloaded as: " << original_filename 
//                   << " (" << total_bytes << " bytes)\r\n";
//               client->sendMessage(oss.str());
//           }
          
//             std::ostringstream oss;
//             oss << "NOTICE :File downloaded as: " << original_filename 
//               << " (" << total_bytes << " bytes)\r\n";
//             client->sendMessage(oss.str());
//       } else {
//           client->sendMessage("NOTICE :File not found: " + server_filename + "\r\n");
//       }


// }
// }



// LoopDeLoop::LoopDeLoop(SocketZilla &_socket, std::string password,
//                        SockItToMe &epoll_instance)
//     : _serverSocket(_socket), _password(password), _poller(epoll_instance) {
//   _poller.addFd(_serverSocket.getFd(), NULL);
// }

// LoopDeLoop::~LoopDeLoop() {
//   for (std::map<int, Client *>::iterator it = _clients.begin();
//        it != _clients.end(); ++it)
//     delete it->second;
// }
// std::vector<std::string> LoopDeLoop::extractLines(std::string &buffer) {
//   std::vector<std::string> lines;
//   size_t pos;
//   while ((pos = buffer.find("\r\n")) != std::string::npos) {
//     lines.push_back(buffer.substr(0, pos));
//     buffer.erase(0, pos + 2);
//   }
//   return lines;
// }



// void LoopDeLoop::handleCommand(Client *client, const std::string &line)
// {
//   std::istringstream iss(line);

//   bot bot;
//   std::string command;
//   iss >> command;

//   if (!command.empty() && command[0] == '/')
//   {
//       bot.bot_handle(client, line);
//       return;
//   }

// //  ayoub the filetransfer
//   else if (command == "XFER")
//   {
//         std::vector<std::string> tokens;; 
        
//         std::string token;
//         while (iss >> token) {
//             tokens.push_back(token);
//         }
//         if ( tokens.size() >= 3)  // FIXED: was token.size(), should be tokens.size()
//           handleFileTransferCommand(client, tokens);
//         return;
//   }
//   else if (command == "PASS") {
//     std::string pass;
//     iss >> pass;
//     pass += "\r\n"; // Ensure CRLF is included
//     client->setPassword(pass);
//   }
//   else if (command == "NICK")
//   {
//     std::string nick;
//     iss >> nick;
//     nick += "\r\n"; // Ensure CRLF is included
//     if (nickExist(nick)) {
//       // FIXED: Proper IRC error format with CRLF
//       std::string err = ":server 433 * " + nick + " :Nickname is already in use\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }
//     client->setNickname(nick);
//     client->setHasNick(true);
//   }
//   else if (command == "USER") {
//     std::string username, unused, unused2, realname;
//     iss >> username >> unused >> unused2;
//     std::getline(iss, realname);
//     if (!realname.empty() && realname[0] == ':')
//       realname.erase(0, 1);
//     username += "\r\n"; // Ensure CRLF is included
//     client->setUsername(username);
//     client->setRealname(realname);
//     client->setHasUser(true);
//   }
//   else if (command == "JOIN") {
//     if (!client->isRegistered()) {
//       // FIXED: Proper IRC error format with CRLF
//       std::string err = ":server 451 " + client->getNickname() + " JOIN :You have not registered\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }
//     std::string channelList;
//     iss >> channelList;

//     if (channelList.empty()) {
//       std::string err = ":server 461 " + client->getNickname() + " JOIN :Not enough parameters\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }

//     std::stringstream ss(channelList);
//     std::string channelName;
//     while (std::getline(ss, channelName, ',')) {
//       if (channelName.empty() || channelName[0] != '#') {
//         std::string err = ":server 476 " + client->getNickname() + " " + channelName + " :Bad Channel Mask\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//         continue;
//       }

//       // Check if already in channel
//       if (client->isInChannel(channelName)) {
//         std::string err = ":server 443 " + client->getNickname() + " " + channelName + " :is already on channel\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//         continue;
//       }

//       Channel *channel = NULL;
//       std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
//       if (it == _channels.end()) {
//         channel = new Channel(channelName);
//         channel->addOperator(client);
//         _channels[channelName] = channel;
//       } else {
//         channel = it->second;
//       }

//       // Check if channel is invite-only
//       if (channel->isInviteOnly() && (!channel->isInvated(client))) {  // Fixed typo: isInvated -> isInvited
//         std::string err = ":server 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//         continue;
//       }

//       // Check if channel has a key
//       if (channel->hasKey()) {
//         std::string key;
//         iss >> key;
//         if (key.empty() || key != channel->getKey()) {
//           std::string err = ":server 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n";
//           send(client->getFd(), err.c_str(), err.size(), 0);
//           continue;
//         }
//       }

//       channel->addClient(client);
//       client->joinChannel(channelName);

//       // Send JOIN message to all channel members including the joiner
//       std::string joinMsg = ":" + client->getNickname() + " JOIN :" + channelName + "\r\n";
//       channel->broadcast(joinMsg, NULL);
      
//       // Send channel topic if it exists
//       if (!channel->getTopic().empty()) {
//         std::string topicMsg = ":server 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
//         send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
//       }
      
//       // Send NAMES list (list of users in channel)
//       std::string namesList = ":server 353 " + client->getNickname() + " = " + channelName + " :";
//       // Add logic to get channel members
//       // namesList += /* channel members */;
//       namesList += "\r\n";
//       send(client->getFd(), namesList.c_str(), namesList.size(), 0);
      
//       std::string endNames = ":server 366 " + client->getNickname() + " " + channelName + " :End of NAMES list\r\n";
//       send(client->getFd(), endNames.c_str(), endNames.size(), 0);
//     }
//   }
//   else if (command == "PRIVMSG") {
//     if (!client->isRegistered()) {
//       // FIXED: Proper IRC error format with nickname and CRLF
//       std::string err = ":server 451 " + client->getNickname() + " PRIVMSG :You have not registered\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }
//     std::string target;
//     iss >> target;

//     std::string message;
//     std::getline(iss, message);
//     if (!message.empty() && message[0] == ':')
//       message.erase(0, 1);

//     if (target.empty() || message.empty()) {
//       // FIXED: Proper IRC error format with server prefix and CRLF
//       std::string err = ":server 461 " + client->getNickname() + " PRIVMSG :Not enough parameters\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }

//     // Sending to a channel
//     if (target[0] == '#') {
//       std::map<std::string, Channel *>::iterator it = _channels.find(target);
//       if (it == _channels.end()) {
//         // FIXED: Proper IRC error format with server prefix and CRLF
//         std::string err = ":server 403 " + client->getNickname() + " " + target + " :No such channel\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//         return;
//       }

//       Channel *channel = it->second;

//       if (!client->isInChannel(target)) {
//         // FIXED: Proper IRC error format with server prefix and CRLF
//         std::string err = ":server 442 " + client->getNickname() + " " + target + " :You're not on that channel\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//         return;
//       }

//       std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
//       channel->broadcast(fullMsg, client);
//     } else {
//       // TODO: implementation of msg user to user
//     }
//   }
//   else if (command == "KICK") {
//     if (!client->isRegistered()) {
//       // FIXED: Proper IRC error format with nickname and CRLF
//       std::string err = ":server 451 " + client->getNickname() + " KICK :You have not registered\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }
//     std::string channelName, targetNick, reason;
//     iss >> channelName >> targetNick;
//     std::getline(iss, reason);
//     if (!reason.empty() && reason[0] == ':')
//       reason.erase(0, 1);
//     if (reason.empty())
//       reason = targetNick;

//     std::map<std::string, Channel *>::iterator chIt = _channels.find(channelName);
//     if (chIt == _channels.end())
//       return;

//     Channel *channel = chIt->second;

//     if (!channel->isOperator(client)) {
//       // FIXED: Proper IRC error format with server prefix and CRLF
//       std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }

//     // Find target client
//     Client *target = NULL;
//     for (std::map<int, Client *>::iterator cit = _clients.begin(); cit != _clients.end(); ++cit) {
//       if (cit->second->getNickname() == targetNick) {
//         target = cit->second;
//         break;
//       }
//     }
//     if (!target || !channel->hasClient(target))
//       return;

//     std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
//     channel->broadcast(kickMsg, NULL);
//     channel->removeClient(target);
//     target->partChannel(channelName);
//   }
//   else if (command == "INVITE") {
//     if (!client->isRegistered()) {
//       // FIXED: Proper IRC error format with nickname and CRLF
//       std::string err = ":server 451 " + client->getNickname() + " INVITE :You have not registered\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }
//     std::string targetNick, channelName;
//     iss >> targetNick >> channelName;

//     std::map<std::string, Channel *>::iterator chIt = _channels.find(channelName);
//     if (chIt == _channels.end())
//       return;

//     Channel *channel = chIt->second;

//     if (!channel->isOperator(client)) {
//       // FIXED: Proper IRC error format with server prefix and CRLF
//       std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }

//     Client *target = NULL;
//     for (std::map<int, Client *>::iterator cit = _clients.begin(); cit != _clients.end(); ++cit) {
//       if (cit->second->getNickname() == targetNick) {
//         target = cit->second;
//         break;
//       }
//     }
//     if (!target)
//       return;

//     channel->addInvited(target);

//     std::string inviteMsg = ":" + client->getNickname() + " INVITE " + targetNick + " " + channelName + "\r\n";
//     send(target->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);
//   } 
//   else if (command == "TOPIC") {
//     if (!client->isRegistered()) {
//       // FIXED: Proper IRC error format with nickname and CRLF
//       std::string err = ":server 451 " + client->getNickname() + " TOPIC :You have not registered\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }
//     std::string channelName;
//     iss >> channelName;

//     std::map<std::string, Channel *>::iterator chIt = _channels.find(channelName);
//     if (chIt == _channels.end())
//       return;

//     Channel *channel = chIt->second;

//     std::string newTopic;
//     std::getline(iss, newTopic);
//     if (!newTopic.empty() && newTopic[0] == ':')
//       newTopic.erase(0, 1);

//     if (newTopic.empty()) {
//       // Just show current topic - FIXED: Added server prefix and CRLF
//       std::string topicMsg = ":server 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
//       send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
//     } else {
//       if (channel->isTopicRestricted() && !channel->isOperator(client)) {
//         // FIXED: Proper IRC error format with server prefix and CRLF
//         std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//         return;
//       }
//       std::cout << "###########" << !channel->isOperator(client) << std::endl;
//       channel->setTopic(newTopic);
//       std::string topicMsg = ":" + client->getNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
//       channel->broadcast(topicMsg, NULL);
//     }
//   } 
//   else if (command == "MODE") {
//     if (!client->isRegistered()) {
//       // FIXED: Added missing registration check with proper error format
//       std::string err = ":server 451 " + client->getNickname() + " MODE :You have not registered\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }
    
//     std::string channelName, modes, param;
//     iss >> channelName >> modes >> param;

//     std::map<std::string, Channel *>::iterator chIt = _channels.find(channelName);
//     if (chIt == _channels.end())
//       return;

//     Channel *channel = chIt->second;

//     if (!channel->isOperator(client)) {
//       // FIXED: Proper IRC error format with server prefix and CRLF
//       std::string err = ":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
//       send(client->getFd(), err.c_str(), err.size(), 0);
//       return;
//     }

//     bool adding = true;
//     for (size_t i = 0; i < modes.size(); ++i) {
//       char m = modes[i];
//       if (m == '+')
//         adding = true;
//       else if (m == '-')
//         adding = false;
//       else if (m == 'i')
//         channel->setInviteOnly(adding);
//       else if (m == 't')
//         channel->setTopicRestricted(adding);
//       else if (m == 'k') {
//         if (adding) {
//           if (param.empty())
//             continue;
//           channel->setHasKey(adding);
//           channel->setKey(param);
//         } else
//           channel->setKey("");
//       } else if (m == 'l') {
//         if (adding)
//           channel->setUserLimit(std::atoi(param.c_str()));
//         else
//           channel->setUserLimit(-1);
//       } else if (m == 'o') {
//         // Give/take operator privilege
//         Client *target = NULL;
//         for (std::map<int, Client *>::iterator cit = _clients.begin(); cit != _clients.end(); ++cit) {
//           if (cit->second->getNickname() == param) {
//             target = cit->second;
//             break;
//           }
//         }
//         if (target && channel->hasClient(target)) {
//           if (adding)
//             channel->addOperator(target);
//           else
//             channel->removeOperator(target);
//         }
//       } 
//       else {
//         // Unknown mode character - FIXED: Added server prefix and CRLF
//         std::string err = ":server 472 " + client->getNickname() + " ";
//         err += m;
//         err += " :is unknown mode char to me\r\n";
//         send(client->getFd(), err.c_str(), err.size(), 0);
//       }
//     }
//   }
//   else {
//     // FIXED: Added server prefix and CRLF for unknown command
//     std::string response = ":server 421 " + client->getNickname() + " " + command + " :Unknown command\r\n";
//     send(client->getFd(), response.c_str(), response.size(), 0);
//   }

//   // FIXED: Moved registration validation to the end and improved logic
//   if (!client->isRegistered() && client->hasNick() && client->hasUser()) {
//     // FIXED: Only check password if one is set
//     if (!_password.empty() && client->getPassword() != _password) {
//       std::string msg = ":server 464 " + client->getNickname() + " :Password incorrect\r\n";
//       send(client->getFd(), msg.c_str(), msg.length(), 0);
//       _poller.removeFd(client->getFd());
//       close(client->getFd());
//       _clients.erase(client->getFd());
//       delete client;
//       return;
//     }

//     client->setRegistered(true);
//     // FIXED: Proper IRC welcome message format
//     std::string welcome = ":server 001 " + client->getNickname() + " :Welcome to the IRC server!\r\n";
//     send(client->getFd(), welcome.c_str(), welcome.length(), 0);
//   }
// }


// // void LoopDeLoop::handleCommand(Client *client, const std::string &line)
// // {
// //   std::istringstream iss(line);

// //   bot bot;
// //   std::string command;
// //   iss >> command;

// //   if (!command.empty() && command[0] == '/')
// //   {
// //       bot.bot_handle(client, line);
// //       return;
// //   }


// // //  ayoub the filetransfer
// //   else if (command == "XFER")
// //   {
// //         std::vector<std::string> tokens;; 
        
// //         std::string token;
// //         while (iss >> token) {
// //             tokens.push_back(token);
// //         }
// //         if ( token.size() >= 3)
// //           handleFileTransferCommand(client, tokens);
// //         return;
// //   }
// //   else if (command == "PASS") {
// //     std::string pass;
// //     iss >> pass;
// //     client->setPassword(pass);
// //   } else if (command == "NICK") {
// //     std::string nick;
// //     iss >> nick;
// //     if (nickExist(nick)) {
// //       std::string err = "nickname already exists";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }
// //     client->setNickname(nick);
// //     client->setHasNick(true);
// //   } else if (command == "USER") {
// //     std::string username, unused, unused2, realname;
// //     iss >> username >> unused >> unused2;
// //     std::getline(iss, realname);
// //     if (!realname.empty() && realname[0] == ':')
// //       realname.erase(0, 1);
// //     client->setUsername(username);
// //     client->setRealname(realname);
// //     client->setHasUser(true);
// //   }
// //   // else if (command == "JOIN") {
// //   //   if (!client->isRegistered()) {
// //   //     std::string err = ":server 451 <JOIN> :You have not registered";
// //   //     send(client->getFd(), err.c_str(), err.size(), 0);
// //   //     return;
// //   //   }
// //   //   std::string channelList;
// //   //   iss >> channelList;

// //   //   if (channelList.empty()) {
// //   //     std::string err =
// //   //         "461 " + client->getNickname() + " JOIN :Not enough parameters\r\n";
// //   //     send(client->getFd(), err.c_str(), err.size(), 0);
// //   //     return;
// //   //   }

// //   //   std::stringstream ss(channelList);
// //   //   std::string channelName;
// //   //   while (std::getline(ss, channelName, ',')) {
// //   //     if (channelName.empty() || channelName[0] != '#') {
// //   //       std::string err = "476 " + client->getNickname() + " " + channelName +
// //   //                         " :Invalid channel name\r\n";
// //   //       send(client->getFd(), err.c_str(), err.size(), 0);
// //   //       continue;
// //   //     }

// //   //     // Check if already in channel
// //   //     if (client->isInChannel(channelName)) {
// //   //       std::string err = "443 " + client->getNickname() + " " + channelName +
// //   //                         " :is already on channel\r\n";
// //   //       send(client->getFd(), err.c_str(), err.size(), 0);
// //   //       continue;
// //   //     }

// //   //     Channel *channel = NULL;
// //   //     std::map<std::string, Channel *>::iterator it =
// //   //         _channels.find(channelName);
// //   //     if (it == _channels.end()) {
// //   //       channel = new Channel(channelName);
// //   //       channel->addOperator(client);
// //   //       _channels[channelName] = channel;
// //   //     } else {
// //   //       channel = it->second;
// //   //     }

// //   //     // check if channel is restricted to inited only
// //   //     if (channel->isInviteOnly() && (!channel->isInvated(client))) {
// //   //       std::string err = "404: " + client->getNickname() + " " + channelName +
// //   //                         " :channel is invite only\r\n";
// //   //       send(client->getFd(), err.c_str(), err.size(), 0);
// //   //       continue;
// //   //     }

// //   //     // chaeck if a channel has pass key
// //   //     if (channel->hasKey()) {
// //   //       std::string key;
// //   //       iss >> key;
// //   //       if (key.empty() || key != channel->getKey()) {
// //   //         std::string err = "475 " + client->getNickname() +
// //   //                           " cant't not join " + channelName + " (+k)";
// //   //         send(client->getFd(), err.c_str(), err.size(), 0);
// //   //         continue;
// //   //       }
// //   //     }

// //   //     channel->addClient(client);
// //   //     client->joinChannel(channelName);

// //   //     // Broadcast join to all channel members
// //   //     std::string joinMsg =
// //   //         ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
// //   //     channel->broadcast(joinMsg, NULL);
// //   //   }
// //   // }
// //   else if (command == "JOIN") {
// //     if (!client->isRegistered()) {
// //       std::string err = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }
// //     std::string channelList;
// //     iss >> channelList;

// //     if (channelList.empty()) {
// //       std::string err = ":server 461 " + client->getNickname() + " JOIN :Not enough parameters\r\n";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }

// //     std::stringstream ss(channelList);
// //     std::string channelName;
// //     while (std::getline(ss, channelName, ',')) {
// //       if (channelName.empty() || channelName[0] != '#') {
// //         std::string err = ":server 476 " + client->getNickname() + " " + channelName + " :Bad Channel Mask\r\n";
// //         send(client->getFd(), err.c_str(), err.size(), 0);
// //         continue;
// //       }

// //       // Check if already in channel
// //       if (client->isInChannel(channelName)) {
// //         std::string err = ":server 443 " + client->getNickname() + " " + channelName + " :is already on channel\r\n";
// //         send(client->getFd(), err.c_str(), err.size(), 0);
// //         continue;
// //       }

// //       Channel *channel = NULL;
// //       std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
// //       if (it == _channels.end()) {
// //         channel = new Channel(channelName);
// //         channel->addOperator(client);
// //         _channels[channelName] = channel;
// //       } else {
// //         channel = it->second;
// //       }

// //       // Check if channel is invite-only
// //       if (channel->isInviteOnly() && (!channel->isInvated(client))) {  // Fixed typo: isInvated -> isInvited
// //         std::string err = ":server 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
// //         send(client->getFd(), err.c_str(), err.size(), 0);
// //         continue;
// //       }

// //       // Check if channel has a key
// //       if (channel->hasKey()) {
// //         std::string key;
// //         iss >> key;
// //         if (key.empty() || key != channel->getKey()) {
// //           std::string err = ":server 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n";
// //           send(client->getFd(), err.c_str(), err.size(), 0);
// //           continue;
// //         }
// //       }

// //       channel->addClient(client);
// //       client->joinChannel(channelName);

// //       // Send JOIN message to all channel members including the joiner
// //       std::string joinMsg = ":" + client->getNickname() + " JOIN :" + channelName + "\r\n";
// //       channel->broadcast(joinMsg, NULL);
      
// //       // Send channel topic if it exists
// //       if (!channel->getTopic().empty()) {
// //         std::string topicMsg = ":server 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
// //         send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
// //       }
      
// //       // Send NAMES list (list of users in channel)
// //       std::string namesList = ":server 353 " + client->getNickname() + " = " + channelName + " :";
// //       // Add logic to get channel members
// //       // namesList += /* channel members */;
// //       namesList += "\r\n";
// //       send(client->getFd(), namesList.c_str(), namesList.size(), 0);
      
// //       std::string endNames = ":server 366 " + client->getNickname() + " " + channelName + " :End of NAMES list\r\n";
// //       send(client->getFd(), endNames.c_str(), endNames.size(), 0);
// //     }
// //   }
// //   else if (command == "PRIVMSG") {
// //     if (!client->isRegistered()) {
// //       std::string err = ":server 451 <PRIVMSG> :You have not registered";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }
// //     std::string target;
// //     iss >> target;

// //     std::string message;
// //     std::getline(iss, message);
// //     if (!message.empty() && message[0] == ':')
// //       message.erase(0, 1);

// //     if (target.empty() || message.empty()) {
// //       std::string err = "461 " + client->getNickname() +
// //                         " PRIVMSG :Not enough parameters\r\n";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }

// //     // Sending to a channel
// //     if (target[0] == '#') {
// //       std::map<std::string, Channel *>::iterator it = _channels.find(target);
// //       if (it == _channels.end()) {
// //         std::string err = "403 " + client->getNickname() + " " + target +
// //                           " :No such channel\r\n";
// //         send(client->getFd(), err.c_str(), err.size(), 0);
// //         return;
// //       }

// //       Channel *channel = it->second;

// //       if (!client->isInChannel(target)) {
// //         std::string err = "442 " + client->getNickname() + " " + target +
// //                           " :You're not on that channel\r\n";
// //         send(client->getFd(), err.c_str(), err.size(), 0);
// //         return;
// //       }

// //       std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target +
// //                             " :" + message + "\r\n";
// //       channel->broadcast(fullMsg, client);
// //     } else {
// //       // TODO: implementation of msg user to user
// //     }
// //   }
// //   else if (command == "KICK") {
// //     if (!client->isRegistered()) {
// //       std::string err = ":server 451 <KICK> :You have not registered";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }
// //     std::string channelName, targetNick, reason;
// //     iss >> channelName >> targetNick;
// //     std::getline(iss, reason);
// //     if (!reason.empty() && reason[0] == ':')
// //       reason.erase(0, 1);
// //     if (reason.empty())
// //       reason = targetNick;

// //     std::map<std::string, Channel *>::iterator chIt =
// //         _channels.find(channelName);
// //     if (chIt == _channels.end())
// //       return;

// //     Channel *channel = chIt->second;

// //     if (!channel->isOperator(client)) {
// //       std::string err =
// //           "482 " + channelName + " :You're not channel operator\r\n";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }

// //     // Find target client
// //     Client *target = NULL;
// //     for (std::map<int, Client *>::iterator cit = _clients.begin();
// //          cit != _clients.end(); ++cit) {
// //       if (cit->second->getNickname() == targetNick) {
// //         target = cit->second;
// //         break;
// //       }
// //     }
// //     if (!target || !channel->hasClient(target))
// //       return;

// //     std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName +
// //                           " " + targetNick + " :" + reason + "\r\n";
// //     channel->broadcast(kickMsg, NULL);
// //     channel->removeClient(target);
// //     target->partChannel(channelName);
// //   }
// //   else if (command == "INVITE") {
// //     if (!client->isRegistered()) {
// //       std::string err = ":server 451 <INVITE> :You have not registered";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }
// //     std::string targetNick, channelName;
// //     iss >> targetNick >> channelName;

// //     std::map<std::string, Channel *>::iterator chIt =
// //         _channels.find(channelName);
// //     if (chIt == _channels.end())
// //       return;

// //     Channel *channel = chIt->second;

// //     if (!channel->isOperator(client)) {
// //       std::string err =
// //           "482 " + channelName + " :You're not channel operator\r\n";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }

// //     Client *target = NULL;
// //     for (std::map<int, Client *>::iterator cit = _clients.begin();
// //          cit != _clients.end(); ++cit) {
// //       if (cit->second->getNickname() == targetNick) {
// //         target = cit->second;
// //         break;
// //       }
// //     }
// //     if (!target)
// //       return;

// //     channel->addInvited(target);

// //     std::string inviteMsg = ":" + client->getNickname() + " INVITE " +
// //                             targetNick + " " + channelName + "\r\n";
// //     send(target->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);
// //   } else if (command == "TOPIC") {
// //     if (!client->isRegistered()) {
// //       std::string err = ":server 451 <TOPIC> :You have not registered";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }
// //     std::string channelName;
// //     iss >> channelName;

// //     std::map<std::string, Channel *>::iterator chIt =
// //         _channels.find(channelName);
// //     if (chIt == _channels.end())
// //       return;

// //     Channel *channel = chIt->second;

// //     std::string newTopic;
// //     std::getline(iss, newTopic);
// //     if (!newTopic.empty() && newTopic[0] == ':')
// //       newTopic.erase(0, 1);

// //     if (newTopic.empty()) {
// //       // Just show current topic
// //       std::string topicMsg = "332 " + client->getNickname() + " " +
// //                              channelName + " :" + channel->getTopic() + "\r\n";
// //       send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);
// //     } else {
// //       if (channel->isTopicRestricted() && !channel->isOperator(client)) {
// //         std::string err =
// //             "482 " + channelName + " :You're not channel operator\r\n";
// //         send(client->getFd(), err.c_str(), err.size(), 0);
// //         return;
// //       }
// //       std::cout << "###########" << !channel->isOperator(client) << std::endl;
// //       channel->setTopic(newTopic);
// //       std::string topicMsg = ":" + client->getNickname() + " TOPIC " +
// //                              channelName + " :" + newTopic + "\r\n";
// //       channel->broadcast(topicMsg, NULL);
// //     }
// //   } else if (command == "MODE") {
// //     std::string channelName, modes, param;
// //     iss >> channelName >> modes >> param;

// //     std::map<std::string, Channel *>::iterator chIt =
// //         _channels.find(channelName);
// //     if (chIt == _channels.end())
// //       return;

// //     Channel *channel = chIt->second;

// //     if (!channel->isOperator(client)) {
// //       std::string err =
// //           "482 " + channelName + " :You're not channel operator\r\n";
// //       send(client->getFd(), err.c_str(), err.size(), 0);
// //       return;
// //     }

// //     bool adding = true;
// //     for (size_t i = 0; i < modes.size(); ++i) {
// //       char m = modes[i];
// //       if (m == '+')
// //         adding = true;
// //       else if (m == '-')
// //         adding = false;
// //       else if (m == 'i')
// //         channel->setInviteOnly(adding);
// //       else if (m == 't')
// //         channel->setTopicRestricted(adding);
// //       else if (m == 'k') {
// //         if (adding) {
// //           if (param.empty())
// //             continue;
// //           channel->setHasKey(adding);
// //           channel->setKey(param);
// //         } else
// //           channel->setKey("");
// //       } else if (m == 'l') {
// //         if (adding)
// //           channel->setUserLimit(std::atoi(param.c_str()));
// //         else
// //           channel->setUserLimit(-1);
// //       } else if (m == 'o') {
// //         // Give/take operator privilege
// //         Client *target = NULL;
// //         for (std::map<int, Client *>::iterator cit = _clients.begin();
// //              cit != _clients.end(); ++cit) {
// //           if (cit->second->getNickname() == param) {
// //             target = cit->second;
// //             break;
// //           }
// //         }
// //         if (target && channel->hasClient(target)) {
// //           if (adding)
// //             channel->addOperator(target);
// //           else
// //             channel->removeOperator(target);
// //         }
// //       } 
// //       else {
// //         // Unknown mode character
// //         std::string err = "472 " + client->getNickname() + " ";
// //         err += m;
// //         err += " :is unknown mode char to me\r\n";
// //         send(client->getFd(), err.c_str(), err.size(), 0);
// //       }
// //     }
// //   }
// //   else {
// //     std::string response = "421 " + command + " :Unknown command\r\n";
// //     send(client->getFd(), response.c_str(), response.size(), 0);
// //   }

// //   // Validate registration
// //   if (!client->isRegistered() && client->hasNick() && client->hasUser()) {
// //     if (client->getPassword() != _password) {
// //       std::string msg = "464 :Password incorrect\r\n";
// //       send(client->getFd(), msg.c_str(), msg.length(), 0);
// //       _poller.removeFd(client->getFd());
// //       close(client->getFd());
// //       _clients.erase(client->getFd());
// //       delete client;
// //       return;
// //     }

// //     client->setRegistered(true);
// //     std::string welcome =
// //         "001 " + client->getNickname() + " :Welcome to the IRC server!\r\n";
// //     send(client->getFd(), welcome.c_str(), welcome.length(), 0);
// //   }
// // }

// void LoopDeLoop::run() {
//   while (true) {
//     std::vector<struct epoll_event> events = _poller.wait();
//     for (size_t i = 0; i < events.size(); ++i) {
//       if (events[i].data.ptr == NULL) {
//         // New connection
//         int clientFd = accept(_serverSocket.getFd(), NULL, NULL);
//         if (clientFd < 0)
//           continue;
//         fcntl(clientFd, F_SETFL, O_NONBLOCK);
//         Client *client = new Client(clientFd);
//         _clients[clientFd] = client;
//         _poller.addFd(clientFd, client);
//         std::cout << "New client accepted: " << clientFd << std::endl;
//       } else {
//         // Existing client
//         Client *client = static_cast<Client *>(events[i].data.ptr);
//         char buf[512];
//         int n = recv(client->getFd(), buf, sizeof(buf) - 1, 0);
//         if (n <= 0) {
//           std::cout << "Client disconnected: " << client->getFd() << std::endl;
//           _poller.removeFd(client->getFd());
//           close(client->getFd());
//           delete client;
//           _clients.erase(client->getFd());
//         } else {
//           buf[n] = '\0';
//           client->getBuffer().append(buf);
//           std::vector<std::string> lines = extractLines(client->getBuffer());
//           for (size_t j = 0; j < lines.size(); ++j) {
//             handleCommand(client, lines[i]);
//           }
//         }
//       }
//     }
//   }
// }
