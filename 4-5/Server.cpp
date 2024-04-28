#include "common.cpp"
#include <signal.h>
#include <string.h>
#include <iostream>

struct Server {
    std::list<Task> task_list;
    Programmer *programmers;

    int last_id = 2;

    Server() {
        programmers = shm->programmers;
    }

    int find_free_programmer() {
        short num_iter = 0;
        int id = 0;
        for (id = last_id + 1; id != last_id; id++, num_iter++) {
            if(num_iter >= NUM_PROGRAMMERS) {
                error_message("Server can not find free programmer");
                break;
            }
            if (id == NUM_PROGRAMMERS) {
                id = 0;
            }
            if (programmers[id].is_free) {
                last_id = id;
                return id;
            }
        }
        return -1;
    }
};

void func(int nsig) {
    close_common_semaphores();
    unlink_all();
    system_message("Goodbye :(");
    exit(1);
}

void f_at() {
    perror("");
}

int main() {
    init();

    Server server;
    system_message("Server has been started");
    Programmer *programmers = shm->programmers;
    shm->server = &server;

    sem_open(sem_not_busy_name, O_RDWR, 0666);
    int not_busy_now;
    system_message("I'm starting to manage the interaction of programmers...");

    sem_t* task_sems[3];
    for(int i = 0; i < NUM_PROGRAMMERS; i++) {
        if((task_sems[i] = sem_open(programmers[i].task_sem_name, O_RDWR, 0666)) == 0) {
            error_message("бля, не могу открыть семафор");
        }
    }

//    signal(SIGQUIT, func);
    while (1) {
        sem_getvalue(not_busy, &not_busy_now);
        printf("not_busy = %d\n", not_busy_now);
        sleep(1);
        if (not_busy_now != 0) {
            int id = server.find_free_programmer();

            if (!programmers[id].is_task_poped) {
                switch (programmers[id].current_task.task_type) {
                    case TaskType::Programming:
                        // push_back - обычная проверка имеет обычный приоритет
                        server.task_list.push_back(Task{Checking, -1, id});
                        system_message("Get a new task: Checking");
                        break;
                    case TaskType::Checking:
                        if (programmers[id].is_correct == false) {
                            // push_front - исправление программы имеет главный приоритет
                            server.task_list.push_front(Task{Fixing, programmers[id].current_task.id_linked, id});
                        }
                        break;
                    case TaskType::Fixing:
                        // push_front - перепроверка имеет наивысший приоритет
                        server.task_list.push_front(Task{Checking, programmers[id].current_task.id_linked, id});
                        break;
                    default:
                        programmers[id].current_task = Task{Waiting, -1, -1};
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

            if (is_new_task) {
                system_message("Я БЛЯТЬ как-то дошёл до сюда!");
                programmers[id].current_task = new_task;
                programmers[id].is_free = false;
                programmers[id].is_task_poped = false;
                programmers[id].is_correct = true;

//                int old_value;
//                int new_value;
//                sem_getvalue(task_sems[id], &old_value);
                sem_post(task_sems[id]);
//                sem_getvalue(task_sems[id], &new_value);
//                printf("ИЗМЕНИЛ значение %d на %d\n", old_value, new_value);

                sem_wait(not_busy);
            }
        }
    }
}