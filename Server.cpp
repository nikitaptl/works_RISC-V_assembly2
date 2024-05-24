#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

char message[STR_SIZE * 2];

void HandleTcpClient(int client1Socket, int client2Socket) {
  char buffer[STR_SIZE];
  int messageSize;

  do {
    memset(buffer, '\0', STR_SIZE);
    if ((messageSize = recv(client1Socket, buffer, STR_SIZE, 0)) < 0) {
      DieWithError("Receiving message from Client №1 failed");
    }
    sprintf(message, "Message: %s. Redirected to client2", buffer);
    server_message(message);
    if (send(client2Socket, buffer, messageSize, 0) != messageSize) {
      DieWithError("Sending message to Client №2 failed");
    }
    if (strcmp(buffer, "The End") == 0) {
      server_message("Received stop signal. Closing connection and exiting...");
      break;
    }
  } while (messageSize > 0);
  close(client1Socket);
  close(client2Socket);
}

int main(int argc, char *argv[]) {
  int serverSocket, client1Socket, client2Socket;
  struct sockaddr_in serverAddress, client1Address, client2Address;
  unsigned short serverPort, client2Port;
  unsigned int client1Length;
  char *client2Ip;

  if (argc != 4) {
    DieWithError("Incorrect number of arguments. Enter server port, client2 port and address");
  }

  serverPort = atoi(argv[1]);
  client2Ip = argv[2];
  client2Port = atoi(argv[3]);

  OpenMySocket(serverAddress, serverSocket, serverPort);
  sprintf(message,
          "Server IP address = %s, port = %d. Wait for client1 connection...",
          inet_ntoa(serverAddress.sin_addr),
          serverPort);
  server_message(message);

  if (listen(serverSocket, MAXPENDING) < 0) {
    DieWithError("listen() failed");
  }

  OpenOutSocket(client2Address, client2Socket, client2Ip, client2Port);
  client1Length = sizeof(client1Address);
  if ((client1Socket = accept(serverSocket, (struct sockaddr *) &client1Address,
                              &client1Length)) < 0) {
    DieWithError("accept() failed");
  }
  sprintf(message,
          "Connected to client1, ip = %s, port = %d. Start to receive messages:",
          inet_ntoa(client1Address.sin_addr),
          client1Address.sin_port);
  server_message(message);

  if (connect(client2Socket, (struct sockaddr *) &client2Address, sizeof(client2Address)) < 0) {
    DieWithError("Connect to Client №2 failed");
  }
  HandleTcpClient(client1Socket, client2Socket);
}