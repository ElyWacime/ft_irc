#include "FileTransfer.hpp"


// FileTransfer::FileTransfer() {}

// std::string FileTransfer::generateKey(const std::string &from, const std::string &to) {
//     return from + "->" + to;
// }

// std::string FileTransfer::generateServerFilename(const std::string &from, const std::string &to, const std::string &original_filename) {
//     return from + "to" + to + "_" + original_filename;
// }

// // void FileTransfer::cleanupTransfer(const std::string &key) {
// //     transfer_buffers_.erase(key);
// //     transfer_filenames_.erase(key);
// //     transfer_accepted_.erase(key);
// // }   

//     ///                       i think it's not needed if i want a history of transfers


// void FileTransfer::handleCommand(Client *client, const std::vector<std::string> &toks) 
// {
//     if (toks.size() < 3 || toks.size() > 3) {
//         std::string err = "Usage for those command is  :" + toks[0] + " <nick> <filename>\r\n";
//         client->sendMessage(err);
//         return;
//     }
//     else
//     {
//         std::string command = toks[0];
//         std::string targetNick = toks[1];
//         std::string filename = toks[2];

//         client *target = 
//             // Check if the file exists
//         struct stat fileStat;
//         if (stat(filename.c_str(), &fileStat) != 0)
//         {
//             std::string err = "File does not exist: " + filename + "\r\n";
//             client->sendMessage(err);
//             return;
//         }

//         if (command == "OFFER" )
//         {
//             std::string key = generateKey(client->getNickname(), targetNick);
//             // Check if the file is already being transferred
//             if (transfer_buffers_.find(key) != transfer_buffers_.end()) {
//                 std::string err = "File transfer already in progress for " + key + "\r\n";
//                 client->sendMessage(err);
//                 return;
//             }
//             transfer_filenames_[key] = filename;
//             transfer_accepted_[key] = false;
//             transfer_buffers_[key] = "";



//         }

//         // Send the file to the target client
//         sendFileTo(client, targetNick, filename);
//     }
// }