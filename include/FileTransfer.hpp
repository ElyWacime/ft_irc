#pragma once

#include "Client.hpp"
#include <string>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <iomanip>

class client;
class FileTransfer
{

    private:
        std::map<std::string, std::string> transfer_buffers_; // key -> file data
        std::map<std::string, std::string> transfer_filenames_; // key -> original filename
        std::map<std::string, bool> transfer_accepted_; // key -> acceptance status

        std::string generateKey(const std::string &from, const std::string &to);
        std::string generateServerFilename(const std::string &from, const std::string &to, const std::string &original_filename);
        void cleanupTransfer(const std::string &key);

    public:
        FileTransfer();
        ~FileTransfer() {};

        void handleCommand(Client *client, const std::vector<std::string> &toks);
        void sendFileTo(Client *client, const std::string &targetNick, const std::string &filename);
        void receiveFileFrom(Client *client, const std::string &fromNick, const std::string &filename);
};


