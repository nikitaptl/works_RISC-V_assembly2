#include "common.cpp"
#include "functions.h"
#include "unistd.h"
#include <sys/wait.h>
#include <string.h>
#include <iostream>

int programmer_id = -1;
sem_t *sem;
char task_sem_name[25];

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGTERM && sig != SIGHUP) {
        return;
    }
    if (programmer_id == -1) {
        exit(10);
    }
    programmer_message("Received a stop signal.", programmer_id);

    if (sem_close(sem) == -1) {
        error_message("Incorrect close of programmer semaphore");
        perror("");
    }
    if (sem_unlink(task_sem_name) == -1) {
        error_message("Incorrect unlink of programmer semaphore");
        exit(-1);
    }

    if(sig == SIGINT || sig == SIGQUIT || sig == SIGHUP) {
        if (shm->server->pid != -1) {
            kill(shm->server->pid, SIGTERM);
        }
        else {
            close_common_semaphores();
            unlink_all();
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
        sleep(1000);
    }

    sprintf(task_sem_name, "/task-semaphore-%d", programmer_id);

    if ((sem = sem_open(task_sem_name, O_CREAT, 0666, 1)) == 0) {
        error_message("Can not create task semaphore!");
        perror("");
        exit(-1);
    }
    programmer_message("Created my own semaphore!", programmer_id);
    sem_post(start);

    Programmer me;

    int num;
    sem_getvalue(sem, &num);
    printf("sem = %d\n", num);

    me.pid = getpid();
    me.id = programmer_id;
    me.task_sem = sem;
    strcpy(me.task_sem_name, task_sem_name);

    me.is_free = false;
    me.is_task_poped = false;
    shm->programmers[programmer_id] = me;
    Programmer *programmers = shm->programmers;

    sem_wait(server_start);
    srand(time(NULL) + me.pid);
    while (1) {
        sem_wait(programmers[programmer_id].task_sem);
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
                printf(BLUE_TEXT "[Programmer %d]" RESET_TEXT "Fixing an error in my program...\n", programmer_id);
                sleep(3 + rand() % 4);
                printf(BLUE_TEXT "[Programmer %d]" RESET_TEXT "Fixed an error in my program! I am submitting it for re-checking.\n",
                       programmer_id);
                break;
        }

        programmers[programmer_id].is_free = true;
        sem_post(not_busy);
    }
}