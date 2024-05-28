#include "common.h"
#include "functions.h"

const char *TaskTypeNames[] = {"Programming", "Checking", "Fixing", "STOP"};
char messageOutput[256];

int Server::find_free_programmer() {
  short num_iter = 0;
  for (int id = last_id + 1; num_iter < NUM_PROGRAMMERS; id++, num_iter++) {
    if (id == NUM_PROGRAMMERS) {
      id = 0;
    }
    if (programmers[id].is_free) {
      last_id = id;
      return id;
    }
  }
  error_message("Can not find free programmer!");
  return -1;
}

Server::Server() {
  programmers = new Programmer[NUM_PROGRAMMERS];
}

Server::Server(Programmer *programmers) {
  this->programmers = programmers;
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

void OpenServerSocket(int servPort, int &servSock, struct sockaddr_in &servAddr) {
  if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    DieWithError("socket() failed");
  }
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(servPort);
  if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
    sprintf(messageOutput, "bind() failed for %s:%d", inet_ntoa(servAddr.sin_addr), servPort);
    DieWithError(messageOutput);
  }
}

void AcceptConnection(int servSock, int& clientSock, struct sockaddr_in &addr, socklen_t &addrLen, char *message) {
  addrLen = sizeof(addr);
  if ((clientSock = accept(servSock, (struct sockaddr *) &addr, &addrLen)) < 0) {
    DieWithError("accept() failed");
  }
  sprintf(message,
          "Accepted connection: %s:%d",
          inet_ntoa(addr.sin_addr),
          ntohs(addr.sin_port));
  server_message(message);
}