#include "common.h"

int sem_id;
int not_busy;
int start;
int server_start;
struct sembuf semaphore;

pthread_mutex_t mutex;
int programmer_id = -1;

char sem_tasks_names[NUM_PROGRAMMERS][25];
char sem_is_free_names[NUM_PROGRAMMERS][25];
char sem_is_correct_names[NUM_PROGRAMMERS][25];

int sem_tasks[NUM_PROGRAMMERS];
int sem_is_free[NUM_PROGRAMMERS];
int sem_is_correct[NUM_PROGRAMMERS];

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
        int is_free = semctl(sem_is_free[id], 0, GETVAL);
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
    key_t key;
    int i;

    if ((key = ftok(".", 'B')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < NUM_PROGRAMMERS; ++i) {
        if ((programmer2server[i] = msgget(key + i, IPC_CREAT | 0666)) == -1) {
            perror("msgget");
            exit(EXIT_FAILURE);
        }
        if ((server2programmer[i] = msgget(key + NUM_PROGRAMMERS + i, IPC_CREAT | 0666)) == -1) {
            perror("msgget");
            exit(EXIT_FAILURE);
        }
    }
}

void init_semaphores() {
    if ((sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create semaphore id");
        perror("semget");
        exit(-1);
    }
    if ((not_busy = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create not_busy semaphore");
        perror("semget");
        exit(-1);
    }
    if ((start = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create start semaphore");
        perror("semget");
        exit(-1);
    }
    if ((server_start = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create server start semaphore");
        perror("semget");
        exit(-1);
    }

    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        if ((sem_tasks[i] = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
            error_message("Can not create task semaphore");
            perror("semget");
            exit(-1);
        }
        if ((sem_is_free[i] = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
            error_message("Can not create is-free semaphore");
            perror("semget");
            exit(-1);
        }
        if ((sem_is_correct[i] = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) == -1) {
            error_message("Can not create is-correct semaphore");
            perror("semget");
            exit(-1);
        }
    }
}

void close_common_semaphores() {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        error_message("Incorrect close of id semaphore");
        perror("semctl");
    }
    if (semctl(not_busy, 0, IPC_RMID) == -1) {
        error_message("Incorrect close of not_busy semaphore");
        perror("semctl");
    }
    if (semctl(start, 0, IPC_RMID) == -1) {
        error_message("Incorrect close of start semaphore");
        perror("semctl");
    }
    if (semctl(server_start, 0, IPC_RMID) == -1) {
        error_message("Incorrect close of server start semaphore");
        perror("semctl");
    }
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        if (semctl(sem_tasks[i], 0, IPC_RMID) == -1) {
            error_message("Incorrect close of task semaphore");
            perror("semctl");
        }
        if (semctl(sem_is_free[i], 0, IPC_RMID) == -1) {
            error_message("Incorrect close of is-free semaphore");
            perror("semctl");
        }
        if (semctl(sem_is_correct[i], 0, IPC_RMID) == -1) {
            error_message("Incorrect close of is-correct semaphore");
            perror("semctl");
        }
    }
}
