#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>

#define BLUE_TEXT "\033[0;34m"
#define RESET_COLOR "\033[0m"

bool stop_generating = false;

void my_sigquit_handler(int nsig) {
    stop_generating = true;
}

int main() {
    // Получение pid_t Client'a
    char shm_pid_name[] = "shared-pid-t";
    pid_t client_pid;
    int shm_pid_id;

    if ((shm_pid_id = shm_open(shm_pid_name, O_RDONLY, 0666)) == -1) {
        printf("Error: shared memory doesn't exist. Firstly start Client process");
        perror("shm_open");
        return 1;
    }

    pid_t* address_pid = mmap(0, sizeof(client_pid), PROT_READ, MAP_SHARED, shm_pid_id, 0);
    if (address_pid == (pid_t *) -1) {
        printf("Error with getting address of shared memory\n");
        perror("mmap error");
        return -1;
    }

    client_pid = *address_pid;
    printf("Got client's pid: %d\n", client_pid);
    kill(client_pid, SIGUSR1);

    // Передача Client'у чисел
    char shm_name[] = "shared-random-number";
    int shm_id, num;

    if ((shm_id = shm_open(shm_name, O_CREAT | O_RDWR, 0666)) == -1) {
        printf("Error with opening or creating shared memory\n");
        perror("shm_open");
        return 1;
    } else {
        printf("Opened shared memory %s, id = %d\n", shm_name, shm_id);
    }

    if (ftruncate(shm_id, sizeof(num)) == -1) {
        printf("Error with setting size of shared memory\n");
        perror("ftruncate");
        return 1;
    }

    srand(time(NULL));
    int *address = mmap(0, sizeof(num), PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0);
    if (address == (int *) -1) {
        printf("Error with getting address of shared memory\n");
        perror("mmap");
        return -1;
    }

    printf("Start generation\n");
    (void) signal(SIGQUIT, my_sigquit_handler);
    while (true) {
        if (stop_generating) {
            printf("\nCtrl+\\ was pressed. Generation is stopped\n");
            kill(client_pid, SIGQUIT);
            break;
        }
        num = rand() % 1000;
        *address = num;
        printf(BLUE_TEXT"[Generation] " RESET_COLOR "Wrote %d to shared memory\n", num);
        sleep(1);
    }
    close(shm_id);
    printf("Closed shared memory\n");

    if (shm_unlink(shm_name) == -1) {
        printf("Error with unlinking shared memory\n");
        perror("shm_unlink");
        return 1;
    }
    printf("Deleted shared memory\n");
    return 0;
}