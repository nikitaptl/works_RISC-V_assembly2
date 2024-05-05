#include "common.h"
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>

Programmer *programmers;
bool are_programmers_running = false;

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGTERM && sig != SIGHUP) {
        return;
    }
    if (sig == SIGINT || sig == SIGQUIT || sig == SIGHUP) {
        if (are_programmers_running) {
            for (int i = 0; i < NUM_PROGRAMMERS; i++) {
                kill(programmers[i].pid, SIGTERM);
            }
            system_message("Sending a stop signal to the programmers. Suspending execution.");
        }
    } else {
        system_message("Server received a stop signal from the programmer.");
    }
    close_common_semaphores();
    unlink_all();

    system_message("Bye!");
    exit(10);
}

int main() {
    init_semaphores();
    init_pipes();
    signal(SIGINT, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sig_handler);

    // Waiting for all programmers to write their pids to pipes
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        semop(server_start, &semaphore, 1);
    }
    int num;
    semctl(server_start, 0, GETVAL, num);
    if (num == 0) {
        system_message("Waiting for all programmers to write their ids to pipes...");
    }
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        semop(server_start, &semaphore, 1);
    }

    programmers = new Programmer[NUM_PROGRAMMERS];
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        Programmer programmer;
        pid_t pid;
        if (read(programmer2server[i], &pid, sizeof(pid_t)) != sizeof(pid_t)) {
            error_message("Can not read pid from programmer");
            perror("read");
            sig_handler(SIGINT);
        }
        programmer.pid = pid;
        programmer.id = i;
        programmers[i] = programmer;
    }
    are_programmers_running = true;

    Server server(programmers);
    system_message("I'm starting to manage the interaction of programmers...");
    // Allow all programmers to start working
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        Task new_task = Task{Programming, -1, -1};
        if (write(server2programmer[i], &new_task, sizeof(Task)) != sizeof(Task)) {
            error_message("Can not write task to programmer");
            perror("write");
            sig_handler(SIGINT);
        }
        semop(sem_tasks[i], &semaphore, 1);
    }

    int not_busy_now;
    while (1) {
        semctl(not_busy, 0, GETVAL, not_busy_now);
        if (not_busy_now != 0) {
            int id = server.find_free_programmer();

            if (!programmers[id].is_task_poped) {
                switch (programmers[id].current_task.task_type) {
                    case TaskType::Programming:
                        // push_back - обычная проверка имеет обычный приоритет
                        server.task_list.push_back(Task{Checking, -1, -1});
                        programmers[id].is_program_checked = false;
                        system_message("Assigned a new task: Checking");
                        break;
                    case TaskType::Checking:
                        int is_correct;
                        semctl(sem_is_correct[id], 0, GETVAL, is_correct);
                        if (is_correct != 0) {
                            // push_front - исправление программы имеет главный приоритет
                            server.task_list.push_front(Task{Fixing, programmers[id].current_task.id_linked, id});
                            system_message("Assigned a new task: Fixing");
                            semop(sem_is_correct[id], &semaphore, 1);
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
                if (write(server2programmer[id], &new_task, sizeof(Task)) != sizeof(Task)) {
                    error_message("Can not write task to programmer");
                    perror("write");
                    sig_handler(SIGINT);
                }
                programmers[id].is_task_poped = false;
                semop(sem_is_free[id], &semaphore, 1);
                semop(not_busy, &semaphore, 1);
            }
        }
    }
}