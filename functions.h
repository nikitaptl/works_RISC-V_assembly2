#ifndef HW10__FUNCTIONS_H_
#define HW10__FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>

#define RED_TEXT "\033[1;31m"
#define BLUE_TEXT "\033[1;34m"
#define CYAN_TEXT "\033[0;36m"
#define RESET_TEXT "\033[0m"
#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define STR_SIZE 128

#define error_message(message) printf(RED_TEXT "[ERROR]" RESET_TEXT " %s\n", message)
#define server_message(message) printf(CYAN_TEXT "[Server]" RESET_TEXT " %s\n", message)
#define client_message(message, id) printf(BLUE_TEXT "[Client %d]" RESET_TEXT " %s\n", id, message)

void DieWithError(const char *message) /* Error handling function */
{
  error_message(message);
  exit(1);
}

void OpenMySocket(struct sockaddr_in &myAddress, int &mySocket, unsigned short myPort) {
  if ((mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    DieWithError("Creating socket failed");
  }
  memset(&myAddress, 0, sizeof(myAddress));
  myAddress.sin_family = AF_INET;
  myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  myAddress.sin_port = htons(myPort);
  if (bind(mySocket, (struct sockaddr *) &myAddress, sizeof(myAddress)) < 0) {
    DieWithError("bind() failed");
  }
}

void OpenOutSocket(struct sockaddr_in &outAddress, int &outSocket, const char *outIp, unsigned short outPort) {
  if ((outSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    DieWithError("Creating socket failed");
  }
  memset(&outAddress, 0, sizeof(outAddress));
  outAddress.sin_family = AF_INET;
  outAddress.sin_addr.s_addr = inet_addr(outIp);
  outAddress.sin_port = htons(outPort);
}

#endif //HW10__FUNCTIONS_H_