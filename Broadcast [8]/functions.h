#ifndef HW10__FUNCTIONS_H_
#define HW10__FUNCTIONS_H_

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RED_TEXT "\033[1;31m"
#define BLUE_TEXT "\033[1;34m"
#define CYAN_TEXT "\033[0;36m"
#define RESET_TEXT "\033[0m"
#define STR_SIZE 256
#define BROADCAST_PORT 60000

#define error_message(message) printf(RED_TEXT "[ERROR]" RESET_TEXT " %s\n", message)
#define server_message(message) printf(CYAN_TEXT "[Server]" RESET_TEXT " %s\n", message)
#define receiver_message(message) printf(BLUE_TEXT "[Receiver]" RESET_TEXT " %s\n", message)

void DieWithError(const char *message) {
  error_message(message);
  exit(1);
}

void OpenReceiverSocket(int &sock, struct sockaddr_in &address, unsigned short port) {
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    DieWithError("socket() failed");
  }
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);
  if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
    perror("");
    DieWithError("bind() failed");
  }
}

void OpenSenderSocket(int &sock, struct sockaddr_in &address, const char* ip, unsigned short port) {
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    DieWithError("socket() failed");
  }
  int broadcastPermission = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission,
                 sizeof(broadcastPermission)) < 0) {
    DieWithError("setsockopt() failed");
  }
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(ip);
  address.sin_port = htons(port);
}

#endif //HW10__FUNCTIONS_H_