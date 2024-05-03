#include "common.h"

key_t task_channel_key;
int task_channel_id;
SharedMemory *shm;

int not_busy_channel_id;
int start_channel_id;
int server_start_channel_id;

Server::Server() {
    programmers = shm->programmers;
}

int Server::find_free_programmer() {
    short num_iter = 0;
    for (int id = last_id + 1; num_iter < NUM_PROGRAMMERS; id++, num_iter++) {
        if (id == NUM_PROGRAMMERS) {
            id = 0;
        }
        if (programmers[id].is_free) {
            last_id = id;
            return id;
        }
    }
    error_message("Can not find free programmer!");
    return -1;
}

// Opens shared memory and common channels
void init() {
    if ((task_channel_key = ftok("/tmp", 'A')) == -1) {
        error_message("Can not generate task channel key");
        perror("ftok");
        exit(-1);
    }

    if ((task_channel_id = msgget(task_channel_key, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create task channel");
        perror("msgget");
        exit(-1);
    }

    if ((not_busy_channel_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create not busy channel");
        perror("msgget");
        exit(-1);
    }

    if ((start_channel_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create start channel");
        perror("msgget");
        exit(-1);
    }

    if ((server_start_channel_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1) {
        error_message("Can not create server start channel");
        perror("msgget");
        exit(-1);
    }
}

void close_common_channels() {
    if (msgctl(not_busy_channel_id, IPC_RMID, NULL) == -1) {
        error_message("Incorrect close of not busy channel");
        perror("msgctl");
    }

    if (msgctl(start_channel_id, IPC_RMID, NULL) == -1) {
        error_message("Incorrect close of start channel");
        perror("msgctl");
    }

    if (msgctl(server_start_channel_id, IPC_RMID, NULL) == -1) {
        error_message("Incorrect close of server start channel");
        perror("msgctl");
    }
}