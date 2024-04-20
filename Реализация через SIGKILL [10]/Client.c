#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>

void sigusr_handler(int signum) {
    printf("Start to receive numbers from the server:");
}

int main() {
    // Передача другому процессу pid_id через разделяемую память
    char shm_pid_name[] = "shared-pid-t";
    pid_t my_pid = getpid();
    int shm_pid_id;
    if ((shm_pid_id = shm_open(shm_pid_name, O_CREAT | O_RDWR, 0666)) == -1) {
        printf("Error with opening shared memory for transmission pid id");
        perror("shm_open");
        return 1;
    }
    if (ftruncate(shm_pid_id, sizeof(my_pid)) == -1) {
        printf("Error with setting size of shared memory!");
        perror("ftruncate");
        return 1;
    }
    pid_t *address_pid = mmap(0, sizeof(pid_t), PROT_WRITE | PROT_READ, MAP_SHARED, shm_pid_id, 0);
    if (address_pid == (pid_t * ) - 1) {
        printf("Error with getting address of shared memory\n");
        perror("mmap");
        return -1;
    }
    *address_pid = my_pid;

    // Ожидание, пока сервер не начнёт генерировать и отправлять числа
    printf("Wait for the server to start generating numbers\n");
    signal(SIGUSR1, sigusr_handler);
    pause();

    // Получение чисел от Server
    char shm_name[] = "shared-random-number";
    int shm_id, num;

    if ((shm_id = shm_open(shm_name, O_RDONLY, 0666)) == -1) {
        printf("Error with opening shared memory\n");
        perror("shm_open");
        return 1;
    } else {
        printf("Opened shared memory %s, id = %d\n", shm_name, shm_id);
    }

    int* address = mmap(0, sizeof(num), PROT_READ, MAP_SHARED, shm_id, 0);
    if (address == (int *) -1) {
        printf("Error with getting address of shared memory\n");
        perror("mmap error");
        return -1;
    }

    // Процесс закончит свою работу, когда получит сигнал SIGKILL
    while (true) {
        num = *address;
        printf("Got %d\n", num);
        sleep(1);
    }
    return 0;
}
