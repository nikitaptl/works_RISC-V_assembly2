#include "common.h"

char shm_name[] = "/posix-shar-object";
int shm_id;
SharedMemory *shm;

char sem_not_busy_name[] = "/not-busy-semaphore";
sem_t *not_busy;

char sem_start_name[] = "/start-semaphore";
sem_t *start;

char sem_server_start_name[] = "/server-start-semaphore";
sem_t *server_start;

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
        exit(-1);
    }

    printf(CYAN_TEXT "[System] " RESET_TEXT "Opened shared memory %s, id = %d\n", shm_name, shm_id);
    if (ftruncate(shm_id, sizeof(SharedMemory)) == -1) {
        error_message("Can not allocate shared memory");
        exit(-1);
    }

    shm = static_cast<SharedMemory *>(mmap(0, sizeof(SharedMemory), PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0));
    if (shm == (SharedMemory *) -1) {
        error_message("Can not receive address of shared memory");
        exit(-1);
    }
    shm->server = new Server();
    shm->server->pid = -1;

    if ((not_busy = sem_open(sem_not_busy_name, O_CREAT, 0666, 0)) == 0) {
        error_message("Can not create not_busy semaphore");
        exit(-1);
    }
    if ((start = sem_open(sem_start_name, O_CREAT, 0666, 0)) == 0) {
        error_message("Can not create start semaphore");
        exit(-1);
    }
    if ((server_start = sem_open(sem_server_start_name, O_CREAT, 0666, 0)) == 0) {
        error_message("Can not create server start semaphore");
        exit(-1);
    }
}

void close_common_semaphores() {
    if (sem_close(not_busy) == -1) {
        error_message("Incorrect close of not_busy semaphore");
        exit(-1);
    }
    if(sem_close(start) == -1) {
        error_message("Incorrect close of start semaphore");
        exit(-1);
    }
    if(sem_close(server_start) == -1) {
        error_message("Incorrect close of server start semaphore");
        exit(-1);
    }
}

void unlink_all() {
    if (shm_unlink(shm_name) == -1) {
        error_message("Incorrect unlink of shared memory");
    }
    if (sem_unlink(sem_not_busy_name) == -1) {
        error_message("Incorrect unlink of not busy semaphore");
    }
}