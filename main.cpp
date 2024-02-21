#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

const size_t bufer_size = 32;

int main(int argc, char *argv[]) {
    const char *path_input = argv[1];
    const char *path_output = argv[2];

    int fd_read;
    if ((fd_read = open(path_input, O_RDONLY)) < 0) {
        printf("Can't open this input file!");
        exit(-1);
    }

    int fd_write;
    if ((fd_write = open(path_output, O_WRONLY | O_CREAT)) < 0) {
        printf("Can't open this output file, or he already exists\n");
        exit(-1);
    }

    char bufer[bufer_size];
    size_t read_bytes;
    do {
        read_bytes = read(fd_read, bufer, bufer_size);
        if (read_bytes == -1) {
            printf("Can't read this file!\n");
            exit(-1);
        }
        write(fd_write, bufer, read_bytes);
    } while (read_bytes == bufer_size);

    printf("The result was succesfully writed to %s\n", path_output);
    if (close(fd_read) < 0) {
        printf("Can't close input file\n");
    }
    if (close(fd_write) < 0) {
        printf("Cant't close output file\n");
    }
    return 0;
}