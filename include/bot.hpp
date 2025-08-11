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

class bot
{
    private:

        std::string joke[10] = {
        "Why did the chicken cross the road? , To get to the other side",
        "What do you call a fish with no eyes? , A fsh",
        }

        std::string quote[10] = {
        "The only way to do great work is to love what you do. - Steve Jobs",
        "The best way to predict the future is to invent it. - Alan Kay",
        "The only way to do great work is to love what you do. - Steve Jobs",
        }

        std::string ascii = "0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel
            8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si
            16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb
            24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us
            32 sp    33  !    34  \"    35  #    36  $    37  %    38  &    39  \'
            40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /
            48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7
            56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?
            64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G
            72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O
            80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W
            88  X    89  Y    90  Z    91  [    92  \\    93  ]    94  ^    95  _
            96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g
            104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o
            112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w
            120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del"

        std::string coin[2] = {
        "Heads",
        "Tails",
        }

        std::string dice[6] = {
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        }


    public:

        std::string get_random_joke()
        {
        std::srand(time(NULL));
        return joke[rand() % 10];
        }

        std::string get_random_quote()
        {
        std::srand(time(NULL));
        return quote[rand() % 10];
        }

        std::string get_random_ascii()
        {
        return ascii;
        }

        std::string get_random_coin()
        {
        std::srand(time(NULL));
        return coin[rand() % 2];
        }

        std::string get_random_dice()
        {
        std::srand(time(NULL));
        return dice[rand() % 6];
        }

        void send_message(Client *client, const std::string &message)
        {
        send(client->getFd(), message.c_str(), message.size(), 0);
        }
    
        void bot_handle(Client *client, const std::string &line)
        {
            std::istringstream iss(line);
            std::string command;
            iss >> command;
            if (command[0] != '/')
                return;
            if (command == "/help")
            {
            std::string help = "Available commands: /joke, /quote, /ascii, /coin, /dice";
            send(client->getFd(), help.c_str(), help.size(), 0);
            return;
            }
            else if (command == "/joke")
            {
            std::string joke = get_random_joke();
            send_message(client, joke);
            return;
            }
            else if (command == "/quote")
            {
            std::string quote = get_random_quote();
            }

            else if (command == "/ascii")
            {
            std::string ascii = get_random_ascii();
            send_message(client, ascii);
            return;
            }
            else if (command == "/coin")
            {
                std::string coin = get_random_coin();
                send_message(client, coin);
                return;
            }
            else if (command == "/dice")
            {
                std::string dice = get_random_dice();
                send_message(client, dice);
                return;
            }
            else
            {
                std::string err = "Unknown command";
                send_message(client, err);
                return;
            }

        }
}
