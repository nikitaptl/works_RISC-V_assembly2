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
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

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
    int task_sem;
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
    bool is_running = false;
};

// Since we use single semaphores, we can perform these simple operations with them
static struct sembuf plus {
        sem_num: 0,
        sem_op: 1,
        sem_flg: 0
};
static struct sembuf minus {
        sem_num: 0,
        sem_op: -1,
        sem_flg: 0
};

extern int programmer_id;
extern char pathname[];
extern char task_pathname[];
extern int shm_id;
extern SharedMemory *shm;

// Semaphores
extern int not_busy;
extern int start;
extern int server_start;

void init();
void unlink_all();
int custom_sem_open(char *pathname, int num);
void custom_sem_destroy(int sem_id);

#endif //IHW2_COMMON_H