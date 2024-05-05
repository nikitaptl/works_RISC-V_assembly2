#include "common.h"
#include <string.h>
#include <signal.h>
#include <time.h>

int programmer_id = -1;

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGHUP) {
        return;
    }
    if (programmer_id != -1) {
        programmer_message("Received a stop signal", programmer_id);
        programmer_message("Bye!", programmer_id);
    } else {
        unlink_all();
        system_message("Received a stop signal");
        system_message("Bye!");
    }
    exit(10);
}

int main() {
    init();
    signal(SIGINT, sig_handler);
    signal(SIGQUIT, sig_handler);
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
        for (int i = 0; i < NUM_PROGRAMMERS; i++) {
            sem_wait(&shm->start);
        }

        Server server;
        system_message("Server has been started");
        Programmer *programmers = shm->programmers;

        system_message("I'm starting to manage the interaction of programmers...");
        // Allow all programmers to start working
        for (int i = 0; i < NUM_PROGRAMMERS; i++) {
            sem_post(&shm->server_start);
        }

        int not_busy_now;
        while (1) {
            sem_getvalue(&shm->not_busy, &not_busy_now);
            if (not_busy_now != 0) {
                int id = server.find_free_programmer();

                if (!programmers[id].is_task_poped) {
                    switch (programmers[id].current_task.task_type) {
                        case TaskType::Programming:
                            // push_back - обычная проверка имеет обычный приоритет
                            server.task_list.push_back(Task{Checking, -1, id});
                            programmers[id].is_program_checked = false;
                            system_message("Assigned a new task: Checking");
                            break;
                        case TaskType::Checking:
                            if (programmers[id].is_correct == false) {
                                // push_front - исправление программы имеет главный приоритет
                                server.task_list.push_front(Task{Fixing, programmers[id].current_task.id_linked, id});
                                system_message("Assigned a new task: Fixing");
                            } else {
                                programmers[programmers[id].current_task.id_linked].is_program_checked = true;
                            }
                            break;
                        case TaskType::Fixing:
                            // push_front - перепроверка имеет наивысший приоритет
                            server.task_list.push_front(Task{Checking, programmers[id].current_task.id_linked, id});
                            system_message("Assigned a new task: Checking");
                            break;
                    }
                    programmers[id].is_task_poped = true;
                }

                Task new_task;
                bool is_new_task = false;
                for (auto it = server.task_list.begin(); it != server.task_list.end(); it++) {
                    if (it->id_performer != id && it->id_performer != -1 || it->id_linked == id) {
                        continue;
                    } else {
                        new_task = *it;
                        server.task_list.erase(it);
                        is_new_task = true;
                        break;
                    }
                }
                if (programmers[id].is_program_checked) {
                    new_task = Task{Programming, -1, -1};
                    system_message("Assigned a new task: Programming");
                    is_new_task = true;
                }

                if (is_new_task) {
                    programmers[id].current_task = new_task;
                    programmers[id].is_free = false;
                    programmers[id].is_task_poped = false;
                    programmers[id].is_correct = true;

                    sem_post(&shm->task_sems[id]);
                    sem_wait(&shm->not_busy);
                }
            }
        }
    }
    sem_post(&shm->start);

    Programmer me;
    me.id = programmer_id;
    me.is_free = false;
    me.is_task_poped = false;
    shm->programmers[programmer_id] = me;
    Programmer *programmers = shm->programmers;

    sem_wait(&shm->server_start);
    srand(time(NULL) + getpid());
    while (1) {
        sem_wait(&shm->task_sems[programmer_id]);
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
        sem_post(&shm->not_busy);
    }
}