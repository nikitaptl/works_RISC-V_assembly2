#include "common.h"

int serverSocket;
char message[256];

void DieWithError(const char *message) {
  error_message(message);
  perror("");
  exit(1);
}

void sig_handler(int sig) {
  if (sig != SIGINT && sig != SIGQUIT && sig != SIGHUP) {
    return;
  }
  observer_message("Received a stop signal. Bye!");
  exit(10);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, sig_handler);
  signal(SIGQUIT, sig_handler);
  signal(SIGHUP, sig_handler);

  struct sockaddr_in serverAddress;
  unsigned short serverPort;
  char *serverIP;

  if (argc < 2 || argc > 4) {
    DieWithError("Wrong number of arguments. Write Server IP and Port(optional)");
  }
  serverIP = argv[1];
  serverPort = argc == 3 ? atoi(argv[2]) : 60000;
  OpenOutSocket(serverAddress, serverSocket, serverIP, serverPort);

  sprintf(message,
          "Ready to receive messages from server, ip = %s, port = %d",
          inet_ntoa(serverAddress.sin_addr),
          serverPort);
  observer_message(message);

  while (1) {
    memset(message, 0, sizeof(message));
    ssize_t recvLen = recvfrom(serverSocket, message, sizeof(message), 0, nullptr, nullptr);
    if (recvLen < 0) {
      DieWithError("Can not read message from server");
    } else if (recvLen == 0) {
      sig_handler(SIGINT);
    }
    observer_message(message);
  }
}
