#include "common.h"

char shm_name[] = "/posix-shar-object";
int shm_id;
SharedMemory *shm;

char sem_not_busy_name[] = "/not-busy-semaphore";
sem_t *not_busy;

char mutex_name[] = "/mutex-semaphore";
sem_t *mutex;

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

    if ((not_busy = sem_open(sem_not_busy_name, O_CREAT, 0666, 0)) == 0) {
        error_message("Can not create not_busy semaphore");
        exit(-1);
    }
    if ((mutex = sem_open(mutex_name, O_CREAT, 0666, 1)) == 0) {
        error_message("Can not create mutex semaphore");
        exit(-1);
    }
}

void close_common_semaphores() {
    if (sem_close(not_busy) == -1) {
        error_message("Incorrect close of not_busy semaphore");
        exit(-1);
    }
    if (sem_close(mutex) == -1) {
        error_message("Incorrect close of mutex semaphore");
        exit(-1);
    }
}

void unlink_all() {
    if (sem_unlink(sem_not_busy_name)) {
        error_message("Incorrect unlink of not busy semaphore");
    }
    if (sem_unlink(mutex_name)) {
        error_message("Incorrect unlink of mutex semaphore");
    }
    if (shm_unlink(shm_name) == -1) {
        error_message("Incorrect unlink of shared memory");
    }
}