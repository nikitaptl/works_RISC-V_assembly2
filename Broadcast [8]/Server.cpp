#include "functions.h"
#define BROADCAST_IP "255.255.255.255"

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in broadcastAddr;
  char str[STR_SIZE];
  char message[STR_SIZE * 2];
  const char* ip = BROADCAST_IP;
  int broadcastPermission;
  unsigned int strLength = STR_SIZE;
  OpenSenderSocket(sock, broadcastAddr, ip, BROADCAST_PORT);

  server_message("Enter messages to broadcast:");
  while (1) {
    memset(str, 0, sizeof(str));
    fgets(str, sizeof(str), stdin);
    str[strlen(str) - 1] = '\0';
    if (sendto(sock, str, strLength, 0, (struct sockaddr *)
        &broadcastAddr, sizeof(broadcastAddr)) != strLength) {
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
