#include "functions.h"

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in address;
  char *ip;
  unsigned short port;
  char str[STR_SIZE];
  char message[STR_SIZE * 2];
  int strLength;
  struct ip_mreq multicastRequest;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <Multicast IP> <Multicast Port>\n", argv[0]);
    exit(1);
  }

  ip = argv[1];
  port = atoi(argv[2]);
  OpenReceiverSocket(sock, address, ip, port, multicastRequest);
  receiver_message("Receiver is ready to receive messages");

  while (1) {
    memset(str, 0, STR_SIZE);
    if ((strLength = recvfrom(sock, str, STR_SIZE, 0, NULL, 0)) < 0) {
      DieWithError("recvfrom() failed");
    }
    str[STR_SIZE - 1] = '\0';
    sprintf(message, "Received: %s", str);
    receiver_message(message);
    if (strcmp(str, "The End") == 0) {
      receiver_message("Received a stop message. Exiting...");
      break;
    }
  }
  close(sock);
  exit(0);
}
