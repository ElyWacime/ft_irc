#pragma once 

#include <cstdint>
#include "Client.hpp"
#include "Server.hpp"
#include <fstream>
#include "sstream"
#include <map>
#include <vector>

class client;

class transfer
{
    private:
        std::string from;
        std::string to;
        std::string filename;
        std::string buffer;
        bool accepted;
    
    public:
        void handle_cmd(client *client, const std::vector<std::string> &token);
        void sned(client *client , const std::string &from , std::string &filename);
        void recievefilefrom(client *client, const std::string &fromm, const std::string &filename);

};

std::map<std::string , transfer> active;

