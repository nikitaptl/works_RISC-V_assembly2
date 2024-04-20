#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    char shm_name[] = "shared-random-number";
    int shm_id, num;

    if ((shm_id = shm_open(shm_name, O_RDONLY, 0666)) == -1) {
        printf("Error with opening shared memory\n");
        perror("shm_open error");
        return 1;
    } else {
        printf("Opened shared memory %s, id = %d\n", shm_name, shm_id);
    }

    int *address = mmap(0, sizeof(num), PROT_READ, MAP_SHARED, shm_id, 0);
    if (address == (int *) -1) {
        printf("Error with getting address of shared memory\n");
        perror("mmap error");
        return -1;
    }

    while (true) {
        num = *address;
        if (num == -1) {
            printf("Server has stopped generation.\n");
            break;
        }
        printf("Got %d\n", num);
        sleep(1);
    }
    close(shm_id);
    return 0;
}
