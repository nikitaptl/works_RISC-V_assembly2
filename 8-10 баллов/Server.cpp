#include "common.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>

Programmer *programmers;
char message[256], observerMessage[256];
bool are_programmers_running = false;
int servSock, observerSock;
std::vector<struct sockaddr_in> observers;
std::mutex observerMutex;
std::queue<std::string> observerMessages;

void DieWithError(const char *message) {
  error_message(message);
  perror("");
  close(servSock);
  close(observerSock);
  exit(1);
}

void sig_handler(int sig) {
  if (sig != SIGINT && sig != SIGQUIT && sig != SIGHUP) {
    error_message("Received an unknown signal");
    return;
  }
  if (are_programmers_running) {
    server_message("Sending a stop signal to the programmers. Suspending execution.");
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
      Task stop_task = Task{STOP, -1, -1};
      sendto(servSock, &stop_task, sizeof(Task), 0, (struct sockaddr *)&programmers[i].echoServAddr, sizeof(programmers[i].echoServAddr));
    }
  }
  close(servSock);
  close(observerSock);
  server_message("Bye!");
  exit(10);
}

void handleObservers(int observerSock) {
  struct sockaddr_in observerAddr;
  socklen_t observerAddrLen = sizeof(observerAddr);

  while (true) {
    char buffer[256];
    int recvLen = recvfrom(observerSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&observerAddr, &observerAddrLen);
    if (recvLen > 0) {
      {
        std::lock_guard<std::mutex> lock(observerMutex);
        observers.push_back(observerAddr);
      }
      sprintf(observerMessage, "Accepted connection from observer, %s:%d",
              inet_ntoa(observerAddr.sin_addr), ntohs(observerAddr.sin_port));
      server_message(observerMessage);
    }

    std::lock_guard<std::mutex> lock(observerMutex);

    while (!observerMessages.empty()) {
      const char *currentMessage = observerMessages.front().c_str();
      for (auto &observer : observers) {
        sendto(observerSock, currentMessage, strlen(currentMessage), 0, (struct sockaddr *)&observer, sizeof(observer));
      }
      observerMessages.pop();
    }
  }
}

int main(int argc, char *argv[]) {
  signal(SIGINT, sig_handler);
  signal(SIGQUIT, sig_handler);
  signal(SIGHUP, sig_handler);

  programmers = new Programmer[NUM_PROGRAMMERS];
  struct sockaddr_in servAddr, clientAddr, observerAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  Server server(programmers);

  if (argc != 2) {
    DieWithError("Incorrect number of arguments. Enter the server port");
    exit(1);
  }

  int servPort = atoi(argv[1]);
  OpenServerSocket(servPort, servSock, servAddr);

  int observerPort = servPort + 1;
  OpenServerSocket(observerPort, observerSock, observerAddr);

  sprintf(message,
          "Server IP address = %s, port = %d. Waiting for %d programmers to connect...",
          inet_ntoa(servAddr.sin_addr),
          servPort,
          NUM_PROGRAMMERS);
  server_message(message);

  for (int i = 0; i < NUM_PROGRAMMERS; ++i) {
    int recvLen = recvfrom(servSock, message, sizeof(message), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (recvLen > 0) {
      programmers[i].id = i;
      programmers[i].sock = servSock;
      programmers[i].echoServAddr = clientAddr;
      sprintf(message, "Programmer connected, %s:%d", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
      server_message(message);
    } else {
      DieWithError("recvfrom() failed");
    }
  }
  server_message("All programmers connected. I am starting to manage the interaction...");

  std::thread observerThread(handleObservers, observerSock);

  for (int i = 0; i < NUM_PROGRAMMERS; i++) {
    Task new_task = Task{Programming, -1, -1};
    sendto(servSock, &new_task, sizeof(Task), 0, (struct sockaddr *)&programmers[i].echoServAddr, sizeof(programmers[i].echoServAddr));
    sprintf(message, "Assigned a new task: Programming to programmer №%d", i + 1);
    server_message(message);
    AddMessageToObservers(message);
  }
  int free_programmers = 0;

  while (1) {
    if (free_programmers != 0) {
      int id = server.find_free_programmer();
      if (!programmers[id].is_task_poped) {
        switch (programmers[id].current_task.task_type) {
          case TaskType::Programming:
            server.task_list.push_back(Task{Checking, -1, id});
            programmers[id].is_program_checked = false;
            break;
          case TaskType::Checking:
            if (programmers[id].is_correct) {
              server.task_list.push_front(Task{Fixing, programmers[id].current_task.id_linked, id});
            } else {
              programmers[programmers[id].current_task.id_linked].is_program_checked = true;
            }
            break;
          case TaskType::Fixing:
            server.task_list.push_front(Task{Checking, programmers[id].current_task.id_linked, id});
            break;
        }
        programmers[id].is_task_poped = true;
      }

      Task new_task;
      bool is_new_task = false;
      for (auto it = server.task_list.begin(); it != server.task_list.end(); it++) {
        if (it->id_performer != id && it->id_performer != -1 || it->id_linked == id) {
          continue;
        } else {
          new_task = *it;
          server.task_list.erase(it);
          is_new_task = true;
          break;
        }
      }
      if (programmers[id].is_program_checked) {
        new_task = Task{Programming, -1, -1};
        is_new_task = true;
      }

      if (is_new_task) {
        programmers[id].current_task = new_task;
        sendto(servSock, &new_task, sizeof(Task), 0, (struct sockaddr *)&programmers[id].echoServAddr, sizeof(programmers[id].echoServAddr));
        sprintf(message, "Assigned a new task: %s to programmer №%d", TaskTypeNames[new_task.task_type], id + 1);
        server_message(message);
        AddMessageToObservers(message);
        programmers[id].is_task_poped = false;
        free_programmers--;
      }
    }
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
      bool is_correct;
      ssize_t bytesReceived = recvfrom(servSock, &is_correct, sizeof(bool), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
      if (bytesReceived == sizeof(bool)) {
        programmers[i].is_correct = is_correct;
        free_programmers++;
        sprintf(message, "Programmer №%d completed the task", i + 1);
        server_message(message);
        AddMessageToObservers(message);
      }
    }
  }
}
