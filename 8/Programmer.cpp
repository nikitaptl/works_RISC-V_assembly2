#include "common.h"
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>

int programmer_id = -1;
int sem_id;
char task_sem_name[25];

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGTERM && sig != SIGHUP) {
        return;
    }
    if (programmer_id == -1) {
        exit(10);
    }
    programmer_message("Received a stop signal", programmer_id);

    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        error_message("Incorrect remove of programmer semaphore");
        perror("semctl");
    }

    if (sig == SIGINT || sig == SIGQUIT || sig == SIGHUP) {
        // Send signal to the server, if his process is running
        if (shm->server != -1) {
            kill(shm->server, SIGTERM);
        } else {
            if (programmer_id == 0) {
                close_common_semaphores();
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
    if ((sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) < 0) {
        error_message("Can not create task semaphore!");
        perror("");
        exit(-1);
    }
    semctl(sem_id, 0, SETVAL, 1);
    semop(sem_id_start, NULL, 0);

    Programmer me;
    me.pid = getpid();
    me.id = programmer_id;
    me.task_sem_id = sem_id;
    strcpy(me.task_sem_name, task_sem_name);
    me.is_free = false;
    me.is_task_poped = false;
    shm->programmers[programmer_id] = me;
    Programmer *programmers = shm->programmers;

    // Waiting for server start...
    int num;
    semctl(sem_id_server_start, 0, GETVAL, num);
    if (num == 0) {
        programmer_message("Waiting for server start...", programmer_id);
    }
    semop(sem_id_server_start, NULL, 0);

    srand(time(NULL) + me.pid);
    while (1) {
        struct sembuf sem_op;
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        semop(me.task_sem_id, &sem_op, 1);

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

        struct sembuf sem_op2;
        sem_op2.sem_num = 0;
        sem_op2.sem_op = 1;
        sem_op2.sem_flg = 0;
        semop(sem_id_not_busy, &sem_op2, 1);
    }
}