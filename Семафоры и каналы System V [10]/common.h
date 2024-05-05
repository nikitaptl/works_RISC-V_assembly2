#ifndef IHW2_COMMON_H
#define IHW2_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <list>
#include <string.h>
#include <pthread.h>
#include <sys/sem.h>
#include "functions.h"
#include <sys/msg.h>

#define NUM_PROGRAMMERS 3

enum TaskType {
    Programming,
    Checking,
    Fixing
};

struct Task {
    TaskType task_type;
    int id_performer;
    int id_linked;

    Task(TaskType task_type = Programming, int id_performer = -1, int id_linked = -1) : task_type(task_type),
                                                                                        id_performer(id_performer),
                                                                                        id_linked(id_linked) {}
};

struct Programmer {
    Task current_task = Task{TaskType::Programming, -1, -1};
    pid_t pid;
    int id;
    bool is_task_poped = false;
    bool is_program_checked = true;
};

struct Server {
    pid_t pid;
    std::list<Task> task_list;
    Programmer *programmers;
    int last_id = NUM_PROGRAMMERS - 1;

    Server();
    Server(Programmer *programmers);

    int find_free_programmer();
};

extern pthread_mutex_t mutex;
extern int programmer_id;
extern char sem_not_busy_name[25];
extern int not_busy;

extern char sem_start_name[25];
extern int start;

extern char sem_server_start_name[25];
extern int server_start;

extern char sem_id_name[15];
extern int sem_id;

extern char sem_tasks_names[NUM_PROGRAMMERS][25];
extern char sem_is_free_names[NUM_PROGRAMMERS][25];
extern char sem_is_correct_names[NUM_PROGRAMMERS][25];

extern int sem_tasks[NUM_PROGRAMMERS];
extern int sem_is_free[NUM_PROGRAMMERS];
extern int sem_is_correct[NUM_PROGRAMMERS];

extern char programmer2server_names[NUM_PROGRAMMERS][25];
extern int programmer2server[NUM_PROGRAMMERS];

extern char server2programmer_names[NUM_PROGRAMMERS][25];
extern int server2programmer[NUM_PROGRAMMERS];

void init_pipes();
void init_semaphores();

void close_common_semaphores();
void unlink_all();

#endif //IHW2_COMMON_H
