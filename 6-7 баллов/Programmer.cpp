#include "common.h"

int serverSocket;
char message[1024];

void DieWithError(const char *message) {
  error_message(message);
  perror("");
  exit(1);
}

void sig_handler(int sig) {
  if (sig != SIGINT && sig != SIGQUIT && sig != SIGHUP) {
    return;
  }
  programmer_message("Received a stop signal. Bye!");
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
    DieWithError("Wrong number of argument. Write Server IP and Port(optional)");
  }
  serverIP = argv[1];
  serverPort = argc == 3 ? atoi(argv[2]) : 60000;
  OpenOutSocket(serverAddress, serverSocket, serverIP, serverPort);

  if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
    DieWithError("Connection to server failed");
  }
  sprintf(message,
          "Connected to server, ip = %s, port = %d",
          inet_ntoa(serverAddress.sin_addr),
          serverPort);
  programmer_message(message);
  while (1) {
    Task task;
    recv(serverSocket, &task, sizeof(Task), 0);
    bool is_correct = false;
    switch (task.task_type) {
      case TaskType::Programming:programmer_message("Programming...");
        sleep(3 + rand() % 5);
        programmer_message("Wrote a program! I am submitting it for review.");
        break;
      case TaskType::Checking:printf(BLUE_TEXT "[Programmer] " RESET_TEXT "Checking %d program...\n", task.id_linked + 1);
        sleep(2 + rand() % 3);
        if (rand() % 2 == 1) {
          is_correct = true;
          printf(BLUE_TEXT "[Programmer] " RESET_TEXT "Found an error in %d program! I am returning it for corrections.\n",
                 task.id_linked + 1);
        } else {
          printf(BLUE_TEXT "[Programmer] " RESET_TEXT "Didn't find an error in %d program.\n", task.id_linked + 1);
        }
        break;
      case TaskType::Fixing:printf(BLUE_TEXT "[Programmer] " RESET_TEXT "Fixing an error in my program...\n");
        sleep(3 + rand() % 4);
        printf(BLUE_TEXT "[Programmer] " RESET_TEXT "Fixed an error in my program! I am submitting it for re-checking.\n");
        break;
      case ::TaskType::STOP:
        sig_handler(SIGINT);
        break;
    }
    sendto(serverSocket, &is_correct, sizeof(bool), 0, (struct sockaddr *) &serverAddress,
           sizeof(serverAddress));
  }
}