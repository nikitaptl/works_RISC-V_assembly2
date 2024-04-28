#ifndef IHW2_COMMON_H
#define IHW2_COMMON_H

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <list>
#include "functions.h"

#define NUM_PROGRAMMERS 3

enum TaskType {
    Programming,
    Checking,
    Fixing,
    Waiting
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
    bool is_free = true;

    char task_sem_name[25];
    sem_t *task_sem;

    bool is_correct;
    bool is_task_poped;
};

// Defined in Server.cpp
struct Server;

struct SharedMemory {
    Programmer programmers[NUM_PROGRAMMERS];
    Server *server;
};

extern char shm_name[];
extern int shm_id;
extern SharedMemory *shm;

extern char sem_not_busy_name[];
extern sem_t *not_busy;

extern char mutex_name[];
extern sem_t *mutex;

void init();

void close_common_semaphores();

void unlink_all();

#endif //IHW2_COMMON_H
