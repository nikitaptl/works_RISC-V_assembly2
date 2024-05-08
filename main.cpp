#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define RED_TEXT "\x1b[31m"
#define CYAN_TEXT "\x1b[36m"
#define RESET_TEXT "\x1b[0m"

pid_t pid;
bool is_sem = false;
bool is_pipe = false;
int fd[2];
int sem_id;

struct sembuf read_plus =
        {.sem_num = 0, .sem_op = 1, .sem_flg = 0};

struct sembuf read_minus =
        {.sem_num = 0, .sem_op = -1, .sem_flg = 0};

struct sembuf write_plus =
        {.sem_num = 1, .sem_op = 1, .sem_flg = 0};

struct sembuf write_minus =
        {.sem_num = 1, .sem_op = -1, .sem_flg = 0};

void delete_sem() {
    if (semctl(sem_id, 0, IPC_RMID, 0) < 0) {
        printf("Can\'t delete semaphore\n");
        exit(-1);
    }
}

void close_pipe() {
    if (close(fd[0]) == -1) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }
    if (close(fd[1]) == -1) {
        printf("Can\'t close pipe\n");
        exit(-1);
    }
}

void sig_handler(int sig_num) {
    if (sig_num != SIGINT && sig_num != SIGHUP) {
        if (is_pipe && pid != 0) {
            close_pipe();
        }
        if (is_sem && pid != 0) {
            delete_sem();
        }
        if (pid == 0) {
            printf(CYAN_TEXT "[CHILD] " RESET_TEXT "Got a stop signal\n");
        } else {
            printf(RED_TEXT "[PARENT] " RESET_TEXT "Got a stop signal\n");
        }
        exit(10);
    }
}

void child(int *pd, int semid) {
    char buffer[40] = "\0";
    int ch = 1;
    while (ch < 1000000000) {
        if(semop(semid, &read_minus, 1) < 0) {
            printf("Can\'t sub 1 from semaphore\n");
            exit(-1);
        }
        memset(buffer, 0, 40);
        read(pd[0], buffer, 40);
        printf(CYAN_TEXT "[CHILD] " RESET_TEXT "%s\n", buffer);
        if(semop(semid, &write_plus, 1) < 0) {
            printf("Can\'t add 1 to semaphore\n");
            exit(-1);
        }
        if(semop(semid, &write_minus, 1) < 0) {
            printf("Can\'t sub 1 from semaphore\n");
            exit(-1);
        }
        memset(buffer, 0, 40);
        sprintf(buffer, "Got message from child №%d", ch++);
        write(pd[1], buffer, 40);
        if(semop(semid, &read_plus, 1) < 0) {
            printf("Can\'t add 1 to semaphore\n");
            exit(-1);
        }
    }
}

void parent(int *pd, int semid) {
    char buffer[40] = "\0";
    int ch = 1;
    while (ch < 1000000000) {
        if(semop(semid, &write_minus, 1) < 0) {
            printf("Can\'t sub 1 from semaphore\n");
            exit(-1);
        }
        memset(buffer, 0, 40);
        sprintf(buffer, "Got message from parent №%d", ch++);
        write(pd[1], buffer, 40);
        if (semop(semid, &read_plus, 1) < 0) {
            printf("Can\'t sub 1 from semaphore\n");
            exit(-1);
        }
        if(semop(semid, &read_minus, 1) < 0) {
            printf("Can\'t sub 1 from semaphore\n");
            exit(-1);
        }
        memset(buffer, 0, 40);
        read(pd[0], buffer, 15);
        printf(RED_TEXT "[PARENT] " RESET_TEXT "%s\n", buffer);
        if(semop(semid, &write_plus, 1) < 0) {
            printf("Can\'t add 1 to semaphore\n");
            exit(-1);
        }
    }
}

int main() {
    signal(SIGINT, sig_handler);
    signal(SIGHUP, sig_handler);
    int fd[2];
    if (pipe(fd) < 0) {
        printf("Pipe error\n");
        return -1;
    }

    char pathname[] = "main.cpp";
    key_t key = ftok(pathname, 0);
    struct sembuf mybuf;

    if ((sem_id = semget(key, 2, 0666 | IPC_CREAT)) < 0) {
        printf("Can\'t create semaphore\n");
        return -1;
    }
    // Read sem
    semctl(sem_id, 0, SETVAL, 0);
    // Write sem
    semctl(sem_id, 1, SETVAL, 1);

    pid = fork();
    if (pid < 0) {
        printf("Fork error\n");
        return -1;
    } else if (pid == 0) {
        child(fd, sem_id);
    } else {
        parent(fd, sem_id);
        wait(0);
        close(fd[0]);
        close(fd[1]);
        if (semctl(sem_id, 0, IPC_RMID, 0) < 0) {
            printf("Can\'t delete semaphore\n");
            return -1;
        }
    }
    // if (semctl(sem_id, 0, IPC_RMID, 0) < 0) {
    //   printf("Can\'t delete semaphore\n");
    //   return -1;
    // }
    return 0;
}
