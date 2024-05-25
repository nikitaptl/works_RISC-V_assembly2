#include "functions.h"
#define BROADCAST_IP "255.255.255.255"

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in address;
  char *ip;
  unsigned short port;
  char str[STR_SIZE];
  unsigned char ttl;

  if ((argc < 3) || (argc > 4)) {
    fprintf(stderr, "Usage:  %s <Multicast Address> <Port> <Send String> [<TTL>]\n",
            argv[0]);
    exit(1);
  }

  ip = argv[1];
  port = atoi(argv[2]);
  ttl = argc == 4 ? atoi(argv[3]) : 1;

  OpenSenderSocket(sock, address, ip, port, ttl);
  server_message("Enter messages to broadcast:");
  while (1) {
    memset(str, 0, sizeof(str));
    fgets(str, sizeof(str), stdin);
    str[strlen(str) - 1] = '\0';
    if (sendto(sock, str, STR_SIZE, 0, (struct sockaddr *)
        &address, sizeof(address)) != STR_SIZE) {
      DieWithError("sendto() sent a different number of bytes than expected");
    }
    if (strcmp(str, "The End") == 0) {
      server_message("Received stop signal. Exiting...");
      break;
    }
  }
  close(sock);
  exit(0);
}
