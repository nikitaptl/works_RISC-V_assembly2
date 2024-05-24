#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

char message[STR_SIZE * 2];

void HandleTcpClient(int mySocket) {
  char buffer[STR_SIZE];
  int messageSize;

  do {
    memset(buffer, '\0', STR_SIZE);
    if ((messageSize = recv(mySocket, buffer, STR_SIZE, 0)) < 0) {
      DieWithError("Receiving message from Client №1 failed");
      perror("");
    }
    sprintf(message, "Received message from server: %s", buffer);
    client_message(message, 2);
    buffer[STR_SIZE - 1] = '\0';
    if(strcmp(buffer, "The End") == 0) {
      client_message("Received stop signal. Closing connection and exiting...", 2);
      break;
    }
  } while (messageSize > 0);
  close(mySocket);
}

int main(int argc, char *argv[]) {
  int mySocket, serverSocket;
  struct sockaddr_in myAddress, serverAddress;
  unsigned short myPort;
  unsigned int serverLength;

  if (argc != 2) {
    myPort = 60001;
  } else {
    myPort = atoi(argv[1]);
  }

  OpenMySocket(myAddress, mySocket, myPort);
  sprintf(message, "My IP address = %s, port = %d. Wait for connection to server...", inet_ntoa(myAddress.sin_addr), myPort);
  client_message(message, 2);

  if (listen(mySocket, MAXPENDING) < 0) {
    DieWithError("listen() failed");
  }

  serverLength = sizeof(myAddress);
  if ((serverSocket = accept(mySocket, (struct sockaddr *) &serverAddress,
                             &serverLength)) < 0) {
    DieWithError("accept() failed");
  }
  sprintf(message, "Connected to server, ip = %s, port = %d", inet_ntoa(serverAddress.sin_addr), serverAddress.sin_port);
  client_message(message, 2);
  HandleTcpClient(serverSocket);
}

