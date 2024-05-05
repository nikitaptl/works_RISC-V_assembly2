#include "common.h"
#include <string.h>
#include <signal.h>
#include <time.h>

void init() {
    init_semaphores();
    semop(sem_id, &semaphore, 1);
    semctl(sem_id, 0, GETVAL, &programmer_id);
}

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGTERM && sig != SIGHUP) {
        return;
    }
    if (programmer_id == -1) {
        exit(10);
    }
    programmer_message("Received a stop signal. Bye!", programmer_id);
    exit(10);
}

int main() {
    init();
    signal(SIGINT, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sig_handler);

    // Waiting for server start...
    int num;
    semctl(start, 0, GETVAL, num);
    if (num == 0) {
        programmer_message("Waiting for server start...", programmer_id);
    }
    semop(start, &semaphore, 1);
    init_pipes();

    pid_t my_pid = getpid();
    if (write(programmer2server[programmer_id], &my_pid, sizeof(pid_t)) != sizeof(pid_t)) {
        error_message("Can not write pid to server");
        perror("write");
        sig_handler(SIGINT);
    }
    semop(server_start, &semaphore, 1);

    srand(time(NULL) + getpid());
    while (1) {
        semop(sem_tasks[programmer_id], &semaphore, 1);
        Task task;
        if (read(server2programmer[programmer_id], &task, sizeof(Task)) != sizeof(Task)) {
            error_message("Can not read task from server");
            perror("read");
            sig_handler(SIGINT);
        }
        switch (task.task_type) {
            case TaskType::Programming:
                programmer_message("Programming...", programmer_id);
                sleep(3 + rand() % 5);
                programmer_message("Wrote a program! I am submitting it for review.", programmer_id);
                break;
            case TaskType::Checking:
                printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Checking %d program...\n", programmer_id,
                       task.id_linked);
                sleep(2 + rand() % 3);
                if (rand() % 2 == 1) {
                    semop(sem_is_correct[programmer_id], &semaphore, 1);
                    printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Found an error in %d program! I am returning it for corrections.\n",
                           programmer_id, task.id_linked);
                } else {
                    printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Didn't find an error in %d program.\n",
                           programmer_id, task.id_linked);
                }
                break;
            case TaskType::Fixing:
                printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Fixing an error in my program...\n", programmer_id);
                sleep(3 + rand() % 4);
                printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Fixed an error in my program! I am submitting it for re-checking.\n",
                       programmer_id);
                break;
        }
        semop(sem_is_free[programmer_id], &semaphore, 1);
        semop(not_busy, &semaphore, 1);
    }
}
