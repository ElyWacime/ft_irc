#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 6666
#define SERVER "127.0.0.1"
#define BUFFER_SIZE 512

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Setup server address struct
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, SERVER, &server_addr.sin_addr);

  // Connect to IRC server
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("connect");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // Send IRC handshake
  write(sockfd, "NICK testuser\r\n", 15);
  write(sockfd, "USER testuser 0 * :Test User\r\n", 30);

  // Join a channel and send a message
  write(sockfd, "JOIN #testchannel\r\n", 20);
  write(sockfd, "PRIVMSG #testchannel :Hello from test client!\r\n", 47);

  // Read server response (basic)
  while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (bytes <= 0)
      break;
    printf("Server: %s", buffer);
  }

  close(sockfd);
  return 0;
}
