#include "common.h"

key_t shm_key = 1234;
int shm_id;
SharedMemory *shm;

key_t sem_key_not_busy = 2345;
int sem_id_not_busy;

key_t sem_key_start = 3456;
int sem_id_start;

key_t sem_key_server_start = 4567;
int sem_id_server_start;

Server::Server() {
    programmers = shm->programmers;
}

int Server::find_free_programmer() {
    short num_iter = 0;
    for (int id = last_id + 1; num_iter < NUM_PROGRAMMERS; id++, num_iter++) {
        if (id == NUM_PROGRAMMERS) {
            id = 0;
        }
        if (programmers[id].is_free) {
            last_id = id;
            return id;
        }
    }
    error_message("Can not find free programmer!");
    return -1;
}

// Opens shared memory and common semaphores
void init() {
    if ((shm_id = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666)) < 0) {
        error_message("Can not create shared memory");
        perror("shmget");
        exit(-1);
    }

    if ((shm = (SharedMemory *)shmat(shm_id, NULL, 0)) == (SharedMemory *) -1) {
        error_message("Can not attach shared memory");
        perror("shmat");
        exit(-1);
    }
    shm->server = -1;

    if ((sem_id_not_busy = semget(sem_key_not_busy, 1, IPC_CREAT | 0666)) < 0) {
        error_message("Can not create not_busy semaphore");
        perror("semget");
        exit(-1);
    }

    if ((sem_id_start = semget(sem_key_start, 1, IPC_CREAT | 0666)) < 0) {
        error_message("Can not create start semaphore");
        perror("semget");
        exit(-1);
    }

    if ((sem_id_server_start = semget(sem_key_server_start, 1, IPC_CREAT | 0666)) < 0) {
        error_message("Can not create server start semaphore");
        perror("semget");
        exit(-1);
    }

    union semun {
        int val;
        struct semid_ds *buf;
        ushort *array;
    } sem_init_val;
    sem_init_val.val = 0;

    if (semctl(sem_id_not_busy, 0, SETVAL, sem_init_val) == -1) {
        error_message("Can not initialize not_busy semaphore");
        perror("semctl");
        exit(-1);
    }

    if (semctl(sem_id_start, 0, SETVAL, sem_init_val) == -1) {
        error_message("Can not initialize start semaphore");
        perror("semctl");
        exit(-1);
    }

    if (semctl(sem_id_server_start, 0, SETVAL, sem_init_val) == -1) {
        error_message("Can not initialize server start semaphore");
        perror("semctl");
        exit(-1);
    }
}

void close_common_semaphores() {
    if (semctl(sem_id_not_busy, 0, IPC_RMID) == -1) {
        error_message("Incorrect close of not_busy semaphore");
        perror("semctl");
    }

    if (semctl(sem_id_start, 0, IPC_RMID) == -1) {
        error_message("Incorrect close of start semaphore");
        perror("semctl");
    }

    if (semctl(sem_id_server_start, 0, IPC_RMID) == -1) {
        error_message("Incorrect close of server start semaphore");
        perror("semctl");
    }
}

void unlink_all() {
    if (shmdt(shm) == -1) {
        error_message("Incorrect detach of shared memory");
        perror("shmdt");
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        error_message("Incorrect remove of shared memory");
        perror("shmctl");
    }
}