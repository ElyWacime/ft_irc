#include "../include/LoopDeLoop.hpp"
#include "../include/SockItToMe.hpp"
#include "../include/SocketZilla.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
    return 1;
  }

  int port = std::atoi(argv[1]);
  std::string password = argv[2];

  try {
    LoopDeLoop server(port, password);
    server.run(); // Start event loop
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
