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
            *address = -1;
            printf("\nCtrl+\\ was pressed. Generation is stopped\n");
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