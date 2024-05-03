#ifndef IHW2_COMMON_H
#define IHW2_COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
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
    int task_sem_id; // ID of the task semaphore
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

extern key_t shm_key;
extern int shm_id;
extern SharedMemory *shm;

extern key_t sem_key_not_busy;
extern int sem_id_not_busy;

extern key_t sem_key_start;
extern int sem_id_start;

extern key_t sem_key_server_start;
extern int sem_id_server_start;

void init();

void close_common_semaphores();

void unlink_all();

#endif //IHW2_COMMON_H