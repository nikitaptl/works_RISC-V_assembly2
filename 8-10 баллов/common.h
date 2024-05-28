#ifndef IHW2_COMMON_H
#define IHW2_COMMON_H

#include "functions.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <list>

#define NUM_PROGRAMMERS 2
#define NUM_OBSERVERS 2

enum TaskType {
  Programming,
  Checking,
  Fixing,
  STOP
};

extern const char *TaskTypeNames[];

struct Task {
  TaskType task_type;
  // id of the programmer who should complete the task (optional)
  int id_performer;
  // id of the programmer, whose task was created, checked or fixed
  int id_linked;

  Task(TaskType task_type = Programming, int id_performer = -1, int id_linked = -1) : task_type(task_type),
                                                                                      id_performer(id_performer),
                                                                                      id_linked(id_linked) {}
};

struct Programmer {
  Task current_task = Task{TaskType::Programming, -1, -1};
  int id = -1;
  bool is_free = true;
  bool is_correct = true;
  bool is_task_poped = false;
  bool is_program_checked = true;
  int sock;
  sockaddr_in echoServAddr;
};

void DieWithError(const char *message);
void OpenOutSocket(struct sockaddr_in &outAddress, int &outSocket, const char *outIp, unsigned short outPort);
void OpenServerSocket(int servPort, int &servSock, struct sockaddr_in &servAddr);
void AcceptConnection(int servSock, int &clientSock, struct sockaddr_in &addr, socklen_t &addrLen, char *message);

struct Server {
  std::list<Task> task_list;
  Programmer *programmers;
  /* since we look at free programmers in a cyclical way,
   * it is necessary to store the index of the last programmer read */
  int last_id = NUM_PROGRAMMERS - 1;

  Server();
  Server(Programmer *programmers);
  int find_free_programmer();
};

#endif //IHW2_COMMON_H