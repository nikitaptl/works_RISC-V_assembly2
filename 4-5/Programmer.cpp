#include "common.cpp"
#include "functions.h"
#include "unistd.h"
#include <sys/wait.h>
#include <string.h>
#include <iostream>

int programmer_id = 0;

void func(int nsig) {
    printf("Goodbye!\n");
    close_common_semaphores();
    unlink_all();
}

int main() {
    sem_unlink(sem_not_busy_name);
    init();
//    signal(SIGQUIT, func);

    int fork_result;
    for(int i = 0; i < NUM_PROGRAMMERS; i++) {
        fork_result = fork();
        if(fork_result == 0) {
            programmer_id = i;
            break;
        }
    }

    if(fork_result > 0) {
        sleep(1000);
    }

    char task_sem_name[25];
    sprintf(task_sem_name, "/task-semaphore-%d", programmer_id);

    sem_t* sem;
    sem_unlink(task_sem_name);
    if( (sem = sem_open(task_sem_name, O_CREAT, 0666, 1)) == 0) {
        error_message("Can not create task semaphore!");
        perror("");
        exit(-1);
    }

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
    Programmer* programmers = shm->programmers;

    srand(time(NULL) + me.pid);
    while(1) {
        sem_wait(programmers[programmer_id].task_sem);
//        programmer_message("Get a new task!", programmer_id);

        switch(programmers[programmer_id].current_task.task_type) {
            case TaskType::Programming:
                programmer_message("Programming...", programmer_id);
                sleep(1 + rand() % 3);
                break;
            case TaskType::Checking:
                printf(BLUE_TEXT "[Programmer %d]" RESET_TEXT "Checking %d programm...\n", programmers[programmer_id].id, programmers[programmer_id].current_task.id_linked);
                sleep(1 + rand() % 2);

                if(rand() % 2) {
                    programmers[programmer_id].is_correct = false;
                    printf(BLUE_TEXT "[Programmer %d]" RESET_TEXT "Found an error in %d programm\n", programmers[programmer_id].id, programmers[programmer_id].current_task.id_linked);
                } else {
                    programmers[programmer_id].is_correct = true;
                    printf(BLUE_TEXT "[Programmer %d]" RESET_TEXT "Didn't find an error in %d programm\n", programmers[programmer_id].id, programmers[programmer_id].current_task.id_linked);
                }
                break;
            case TaskType::Fixing:
                printf(BLUE_TEXT "[Programmer %d]" RESET_TEXT "Fixing %d programm\n", programmers[programmer_id].id, programmers[programmer_id].current_task.id_linked);
                sleep(1 + rand() % 3);
                break;
            case TaskType::Waiting:
                break;
        }

        programmers[programmer_id].is_free = true;
        sem_post(not_busy);

        int not_busy_now;
        sem_getvalue(not_busy, &not_busy_now);
        printf("not_busy = %d\n", not_busy_now);
    }
}