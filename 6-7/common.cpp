#include "common.h"

char shm_name[] = "/posix-shar-object";
int shm_id;
SharedMemory *shm;

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
    if ((shm_id = shm_open(shm_name, O_CREAT | O_RDWR, 0666)) == -1) {
        error_message("Can not open or create shared memory");
        perror("shm_open");
        exit(-1);
    }

    printf(CYAN_TEXT "[System] " RESET_TEXT "Opened shared memory %s, id = %d\n", shm_name, shm_id);
    if (ftruncate(shm_id, sizeof(SharedMemory)) == -1) {
        error_message("Can not allocate shared memory");
        perror("ftruncate");
        exit(-1);
    }

    shm = static_cast<SharedMemory *>(mmap(0, sizeof(SharedMemory), PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0));
    if (shm == (SharedMemory *) -1) {
        error_message("Can not receive address of shared memory");
        perror("mmap");
        exit(-1);
    }
    if (sem_init(&shm->not_busy, 1, 0) == -1) {
        error_message("Can not create not_busy semaphore");
        perror("sem_init");
        exit(-1);
    }
    if (sem_init(&shm->start, 1, 0) == -1) {
        error_message("Can not create start semaphore");
        perror("sem_init");
        exit(-1);
    }
    if (sem_init(&shm->server_start, 1, 0) == -1) {
        error_message("Can not create server start semaphore");
        perror("sem_init");
        exit(-1);
    }
    for(int i = 0; i < NUM_PROGRAMMERS; i++) {
        if (sem_init(&shm->task_sems[i], 1, 1) == -1) {
            error_message("Can not create task semaphore");
            perror("sem_init");
            exit(-1);
        }
    }
}

void unlink_all() {
    if (shm_unlink(shm_name) == -1) {
        error_message("Incorrect unlink of shared memory");
        perror("shm_unlink");
    }
}