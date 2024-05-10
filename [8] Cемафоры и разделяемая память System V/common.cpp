#include "common.h"

int programmer_id = -1;
char pathname[] = "common.cpp";
key_t shm_key;
int shm_id;
SharedMemory *shm;

key_t sem_not_busy_key;
int not_busy;

key_t sem_start_key;
int start;

key_t sem_server_start_key;
int server_start;

Server::Server() {
    programmers = shm->programmers;
}

int custom_sem_open(char *pathname, int num = 0) {
    key_t key;
    int sem_id;
    if ((key = ftok(pathname, num)) == -1) {
        error_message("Can not create key!");
        perror("ftok");
        exit(-1);
    }
    if ((sem_id = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        if ((sem_id = semget(key, 1, 0)) < 0) {
            error_message("Can not create or connect to semaphore");
            perror("semget");
            exit(-1);
        };
    }
    return sem_id;
}

void custom_sem_destroy(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) < 0) {
        error_message("Can not destroy semaphore");
        perror("");
    }
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
    shm_key = ftok(pathname, 0);
    if (shm_key == -1) {
        error_message("Can not create shm-key!");
        perror("ftok");
        exit(-1);
    }
    if ((shm_id = shmget(shm_key, sizeof(SharedMemory), 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        if ((shm_id = shmget(shm_key, sizeof(SharedMemory), 0)) < 0) {
            error_message("Can not open or create shared memory!");
            perror("");
            exit(-1);
        }
    }
    printf(CYAN_TEXT "[System] " RESET_TEXT "Opened shared memory: id = %d\n", shm_id);
    if ((shm = static_cast<SharedMemory *>(shmat(shm_id, NULL, 0))) == (SharedMemory *) -1) {
        error_message("Can not receive address of shared memory");
        perror("shmat");
        exit(-1);
    }
    shm->server = -1;
    not_busy = custom_sem_open(pathname, 1);
    start = custom_sem_open(pathname, 2);
    server_start = custom_sem_open(pathname, 3);

    if (!shm->is_running) {
        semctl(not_busy, 0, SETVAL, 0);
        semctl(start, 0, SETVAL, 0);
        semctl(server_start, 0, SETVAL, 0);
        shm->is_running = true;
    }
}

void unlink_all() {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        error_message("Incorrect unlink of shared memory");
        perror("shmctl");
    }
    custom_sem_destroy(not_busy);
    custom_sem_destroy(start);
    custom_sem_destroy(server_start);
}