#include "common.h"

pthread_mutex_t mutex;
int programmer_id = -1;
char sem_not_busy_name[] = "/not-busy-semaphore";
sem_t *not_busy;

char sem_start_name[] = "/start-semaphore";
sem_t *start;

char sem_server_start_name[] = "/server-start-semaphore";
sem_t *server_start;

char sem_id_name[] = "/id-semaphore";
sem_t *sem_id;

char sem_tasks_names[NUM_PROGRAMMERS][25];
char sem_is_free_names[NUM_PROGRAMMERS][25];
char sem_is_correct_names[NUM_PROGRAMMERS][25];

sem_t *sem_tasks[NUM_PROGRAMMERS];
sem_t *sem_is_free[NUM_PROGRAMMERS];
sem_t *sem_is_correct[NUM_PROGRAMMERS];

char programmer2server_names[NUM_PROGRAMMERS][25];
int programmer2server[NUM_PROGRAMMERS];

char server2programmer_names[NUM_PROGRAMMERS][25];
int server2programmer[NUM_PROGRAMMERS];

int Server::find_free_programmer() {
    short num_iter = 0;
    for (int id = last_id + 1; num_iter < NUM_PROGRAMMERS; id++, num_iter++) {
        if (id == NUM_PROGRAMMERS) {
            id = 0;
        }
        int is_free;
        sem_getvalue(sem_is_free[id], &is_free);
        if (is_free) {
            last_id = id;
            return id;
        }
    }
    error_message("Can not find free programmer!");
    return -1;
}

Server::Server() {
    programmers = new Programmer[NUM_PROGRAMMERS];
}

Server::Server(Programmer *programmers) {
    this->programmers = programmers;
}

void init_pipes() {
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        if (i != programmer_id && programmer_id != -1) {
            continue;
        }
        sprintf(programmer2server_names[i], "p2s-%d.fifo", i);
        sprintf(server2programmer_names[i], "s2p-%d.fifo", i);
        if (programmer_id == -1) {
            mknod(programmer2server_names[i], S_IFIFO | 0666, 0);
            mknod(server2programmer_names[i], S_IFIFO | 0666, 0);
        }
        if ((programmer2server[i] = open(programmer2server_names[i], O_RDWR)) == -1) {
            error_message("Can not open programmer2server pipe");
            perror("open");
            exit(-1);
        }
        if ((server2programmer[i] = open(server2programmer_names[i], O_RDWR)) == -1) {
            error_message("Can not open server2programmer pipe");
            perror("open");
            exit(-1);
        }
    }
}

void init_semaphores() {
    if ((sem_id = sem_open(sem_id_name, O_CREAT, 0666, 3)) == 0) {
        error_message("Can not create id semaphore");
        perror("sem_open");
        exit(-1);
    }
    if ((not_busy = sem_open(sem_not_busy_name, O_CREAT, 0666, 0)) == 0) {
        error_message("Can not create not_busy semaphore");
        perror("sem_open");
        exit(-1);
    }
    if ((start = sem_open(sem_start_name, O_CREAT, 0666, 0)) == 0) {
        error_message("Can not create start semaphore");
        perror("sem_open");
        exit(-1);
    }
    if ((server_start = sem_open(sem_server_start_name, O_CREAT, 0666, 0)) == 0) {
        error_message("Can not create server start semaphore");
        perror("sem_open");
        exit(-1);
    }

    char buffer[25];
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        sprintf(buffer, "/task-semaphore-%d", i);
        strcpy(sem_tasks_names[i], buffer);
        if ((sem_tasks[i] = sem_open(buffer, O_CREAT, 0666, 0)) == 0) {
            error_message("Can not create task semaphore");
            perror("sem_open");
            exit(-1);
        }

        sprintf(buffer, "/is-free-semaphore-%d", i);
        strcpy(sem_is_free_names[i], buffer);
        if ((sem_is_free[i] = sem_open(buffer, O_CREAT, 0666, 0)) == 0) {
            error_message("Can not create is-free start semaphore");
            perror("sem_open");
            exit(-1);
        }

        sprintf(buffer, "/is-correct-semaphore-%d", i);
        strcpy(sem_is_correct_names[i], buffer);
        if ((sem_is_correct[i] = sem_open(buffer, O_CREAT, 0666, 0)) == 0) {
            error_message("Can not create is-correct semaphore");
            perror("sem_open");
            exit(-1);
        }
    }
}

void close_common_semaphores() {
    if (sem_close(sem_id) == -1) {
        error_message("Incorrect close of id semaphore");
        perror("sem_close");
    }
    if (sem_close(not_busy) == -1) {
        error_message("Incorrect close of not_busy semaphore");
        perror("sem_close");
    }
    if (sem_close(start) == -1) {
        error_message("Incorrect close of start semaphore");
        perror("sem_close");
    }
    if (sem_close(server_start) == -1) {
        error_message("Incorrect close of server start semaphore");
        perror("sem_close");
    }
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        if (sem_close(sem_tasks[i]) == -1) {
            error_message("Incorrect close of task semaphore");
            perror("sem_close");
        }
        if (sem_close(sem_is_free[i]) == -1) {
            error_message("Incorrect close of is-free semaphore");
            perror("sem_close");
        }
        if (sem_close(sem_is_correct[i]) == -1) {
            error_message("Incorrect close of is-correct semaphore");
            perror("sem_close");
        }
    }
}

void unlink_all() {
    if(sem_unlink(sem_id_name) == -1) {
        error_message("Incorrect unlink of id semaphore");
        perror("sem_unlink");
    }
    if (sem_unlink(sem_not_busy_name) == -1) {
        error_message("Incorrect unlink of not busy semaphore");
        perror("sem_unlink");
    }
    if (sem_unlink(sem_start_name) == -1) {
        error_message("Incorrect unlink of start semaphore");
        perror("sem_unlink");
    }
    if (sem_unlink(sem_server_start_name) == -1) {
        error_message("Incorrect unlink of server start semaphore");
        perror("sem_unlink");
    }
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        if (sem_unlink(sem_tasks_names[i]) == -1) {
            error_message("Incorrect unlink of task semaphore");
            perror("sem_unlink");
        }
        if (sem_unlink(sem_is_free_names[i]) == -1) {
            error_message("Incorrect unlink of is-free semaphore");
            perror("sem_unlink");
        }
        if (sem_unlink(sem_is_correct_names[i]) == -1) {
            error_message("Incorrect unlink of is-correct semaphore");
            perror("sem_unlink");
        }
        if (unlink(programmer2server_names[i]) == -1) {
            error_message("Incorrect unlink of server fifo");
            perror("unlink");
        }
        if (unlink(server2programmer_names[i]) == -1) {
            error_message("Incorrect unlink of server fifo");
            perror("unlink");
        }
    }
}