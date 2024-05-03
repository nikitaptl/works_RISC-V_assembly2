#ifndef IHW2_COMMON_H
#define IHW2_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <list>
#include "functions.h"

#define NUM_PROGRAMMERS 3

enum TaskType {
    Programming,
    Checking,
    Fixing
};

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
    pid_t pid;
    int id;
    int task_channel[2];
    bool is_free = true;
    bool is_correct;
    bool is_task_poped;
    bool is_program_checked = true;
};

struct Server {
    pid_t pid;
    std::list<Task> task_list;
    Programmer *programmers;
    /* since we look at free programmers in a cyclical way,
     * it is necessary to store the index of the last programmer read */
    int last_id = 2;

    Server();

    int find_free_programmer();
};

struct SharedMemory {
    Programmer programmers[NUM_PROGRAMMERS];
    pid_t server;
};

extern key_t task_channel_key;
extern int task_channel_id;
extern SharedMemory *shm;

extern int not_busy_channel_id;
extern int start_channel_id;
extern int server_start_channel_id;

void init();

void close_common_channels();

#endif //IHW2_COMMON_H