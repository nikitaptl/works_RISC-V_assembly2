#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

int main(int argc, char *argv[]) {
  int serverSocket;
  struct sockaddr_in serverAddress;
  unsigned short serverPort;
  char *serverIP;
  char str[STR_SIZE];

  if (argc < 2 || argc > 4) {
    DieWithError("Wrong number of argument. Write Server IP and Port(optional)");
  }
  serverIP = argv[1];
  serverPort = argc == 3 ? atoi(argv[2]) : 60000;
  OpenOutSocket(serverAddress, serverSocket, serverIP, serverPort);

  if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    DieWithError("Connect to server failed");
  }

  client_message("Connected to server. Write messages: ", 1);
  while (1) {
    memset(str, 0, sizeof(str));
    fgets(str, sizeof(str), stdin);
    str[strlen(str) - 1] = '\0';
    if (send(serverSocket, str, sizeof(str), 0) != sizeof(str)) {
      DieWithError("send() sent a different number of bytes than expected");
    }
    if(strcmp(str, "The End") == 0) {
      client_message("Received stop signal. Closing connection and exiting...", 1);
      break;
    }
  }
  close(serverSocket);
  exit(0);
}
