#include "common.h"
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>

int programmer_id = -1;
int task_channel_id;
char task_channel_name[25];

void sig_handler(int sig) {
    if (sig != SIGINT && sig != SIGQUIT && sig != SIGTERM && sig != SIGHUP) {
        return;
    }
    if (programmer_id == -1) {
        exit(10);
    }
    programmer_message("Received a stop signal", programmer_id);

    if (sig == SIGINT || sig == SIGQUIT || sig == SIGHUP) {
        // Send signal to the server if its process is running
        if (shm->server != -1) {
            kill(shm->server, SIGTERM);
        } else {
            if (programmer_id == 0) {
                close_common_channels();
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
    sprintf(task_channel_name, "/task-channel-%d", programmer_id);
    if ((task_channel_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create task channel!");
        perror("");
        exit(-1);
    }
    struct msqid_ds channel_info;
    if (msgctl(task_channel_id, IPC_STAT, &channel_info) == -1) {
        error_message("Can not get task channel info!");
        perror("");
        exit(-1);
    }
    key_t task_channel_key = channel_info.msg_perm.__key;
    if (msgsnd(start_channel_id, &task_channel_key, sizeof(key_t), 0) == -1) {
        error_message("Can not send task channel key!");
        perror("");
        exit(-1);
    }

    Programmer me;
    me.pid = getpid();
    me.id = programmer_id;
    me.task_channel_id = task_channel_id;
    strcpy(me.task_channel_name, task_channel_name);
    me.is_free = false;
    me.is_task_poped = false;
    shm->programmers[programmer_id] = me;
    Programmer *programmers = shm->programmers;

    // Wait for server start
    key_t server_start_key;
    if (msgrcv(server_start_channel_id, &server_start_key, sizeof(key_t), 0, 0) == -1) {
        error_message("Can not receive server start key!");
        perror("");
        exit(-1);
    }
    int server_start_channel;
    if ((server_start_channel = msgget(server_start_key, 0666)) == -1) {
        error_message("Can not open server start channel!");
        perror("");
        exit(-1);
    }

    srand(time(NULL) + me.pid);
    while (1) {
        struct msgbuf {
            long mtype;
            Task task;
        } message;

        if (msgrcv(task_channel_id, &message, sizeof(Task), 0, 0) == -1) {
            error_message("Error receiving task message!");
            perror("");
            exit(-1);
        }
        programmers[programmer_id].current_task = message.task;

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

        // Notify server
        if (msgsnd(not_busy_channel_id, &message, sizeof(Task), 0) == -1) {
            error_message("Error sending task completion message!");
            perror("");
            exit(-1);
        }
    }
}