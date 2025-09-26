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
#include <sstream>
#include <unistd.h>
#include <ctime>

class bot
{
private:
    std::string joke[2];
    std::string quote[3];
    std::string ascii;
    std::string coin[2];
    std::string dice[6];

public:
    bot()
    {
        std::srand(std::time(NULL));

        joke[0] = "Why did the chicken cross the road? , To get to the other side";
        joke[1] = "What do you call a fish with no eyes? , A fsh";

        quote[0] = "The only way to do great work is to love what you do. - Steve Jobs";
        quote[1] = "The best way to predict the future is to invent it. - Alan Kay";
        quote[2] = "The only way to do great work is to love what you do. - Steve Jobs";


        coin[0] = "Heads";
        coin[1] = "Tails";

        dice[0] = "1";
        dice[1] = "2";
        dice[2] = "3";
        dice[3] = "4";
        dice[4] = "5";
        dice[5] = "6";
    }


    std::string get_random_joke()
    {
        return joke[std::rand() % 2];
    }

    std::string get_random_quote()
    {
        return quote[std::rand() % 3];
    }

    std::string get_random_coin()
    {
        return coin[std::rand() % 2];
    }

    std::string get_random_dice()
    {
        return dice[std::rand() % 6];
    }

    void send_message(Client *client, const std::string &message)
    {
        send(client->getFd(), message.c_str(), message.size(), 0);
    }

    
};