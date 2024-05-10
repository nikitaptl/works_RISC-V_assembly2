#include "common.h"
#include <string.h>
#include <signal.h>
#include <time.h>

char task_pathname[] = "Programmer.cpp";
int sem = -1;
char task_sem_name[30];

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGTERM && sig != SIGHUP) {
        return;
    }
    if (programmer_id == -1) {
        exit(10);
    }
    programmer_message("Received a stop signal", programmer_id);

    if(sem != -1) {
        custom_sem_destroy(sem);
    }
    if (sig == SIGINT || sig == SIGQUIT || sig == SIGHUP) {
        // Send signal to the server, if his process is running
        if (shm->server != -1) {
            kill(shm->server, SIGTERM);
        } else {
            if (programmer_id == 0) {
                unlink_all();
            }
        }
    }
    programmer_message("Bye!", programmer_id);
    exit(10);
}

int main() {
    init();
    signal(SIGINT, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sig_handler);

    int fork_result;
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        fork_result = fork();
        if (fork_result == 0) {
            programmer_id = i;
            break;
        }
    }
    if (fork_result > 0) {
        pause();
    }
    sprintf(task_sem_name, "/task-semaphore-%d", programmer_id);
    sem = custom_sem_open(task_pathname, programmer_id);
    semctl(sem, 0, SETVAL, 1);

    Programmer me;
    me.pid = getpid();
    me.id = programmer_id;
    me.task_sem = sem;
    me.is_free = false;
    me.is_task_poped = false;
    shm->programmers[programmer_id] = me;
    Programmer *programmers = shm->programmers;
    semop(start, &plus, 1);

    // Waiting for server start...
    int num = semctl(server_start, 0, GETVAL);
    if (num == 0) {
        programmer_message("Waiting for server start...", programmer_id);
    }
    semop(server_start, &minus, 1);

    srand(time(NULL) + me.pid);
    while (1) {
        semop(sem, &minus, 1);
        switch (programmers[programmer_id].current_task.task_type) {
            case TaskType::Programming:
                programmer_message("Programming...", programmer_id);
                sleep(3 + rand() % 5);
                programmer_message("Wrote a program! I am submitting it for review.", programmer_id);
                break;
            case TaskType::Checking:
                printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Checking %d program...\n", programmer_id,
                       programmers[programmer_id].current_task.id_linked);
                sleep(2 + rand() % 3);
                if (rand() % 2 == 1) {
                    programmers[programmer_id].is_correct = false;
                    printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Found an error in %d program! I am returning it for corrections.\n",
                           programmer_id, programmers[programmer_id].current_task.id_linked);
                } else {
                    programmers[programmer_id].is_correct = true;
                    printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Didn't find an error in %d program.\n",
                           programmer_id, programmers[programmer_id].current_task.id_linked);
                }
                break;
            case TaskType::Fixing:
                printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Fixing an error in my program...\n", programmer_id);
                sleep(3 + rand() % 4);
                printf(BLUE_TEXT "[Programmer %d] " RESET_TEXT "Fixed an error in my program! I am submitting it for re-checking.\n",
                       programmer_id);
                break;
        }
        programmers[programmer_id].is_free = true;
        semop(not_busy, &plus, 1);
    }
}