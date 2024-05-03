#include "common.h"
#include <signal.h>
#include <sys/msg.h>

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGTERM && sig != SIGHUP) {
        return;
    }
    if (sig == SIGINT || sig == SIGQUIT || sig == SIGTERM) {
        for (int i = 0; i < NUM_PROGRAMMERS; i++) {
            kill(shm->programmers[i].pid, SIGTERM);
        }
        system_message("Sending a stop signal to the programmers. Suspending execution.");
    } else {
        system_message("Server received a stop signal from the programmer.");
    }
    close_common_channels();
    unlink_all();

    system_message("Bye!");
    exit(10);
}

int main() {
    init();
    signal(SIGINT, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sig_handler);

    // Waiting for all programmers to start and create their channels
    int num;
    msgrcv(start_channel_id, NULL, 0, 0, MSG_NOERROR);
    system_message("All the programmers have started.");

    Server server;
    server.pid = getpid();
    system_message("Server has been started");
    Programmer *programmers = server.programmers;

    system_message("I'm starting to manage the interaction of programmers...");
    // Allow all programmers to start working
    for (int i = 0; i < NUM_PROGRAMMERS; i++) {
        msgsnd(server_start_channel_id, NULL, 0, IPC_NOWAIT);
    }

    while (1) {
        sem_getvalue(not_busy, &not_busy_now);
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

                sem_post(task_sems[id]);
                sem_wait(not_busy);
            }
        }
    }
}