#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

const int buf_size = 5000;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Number of start arguments is incorrect\n");
        exit(-1);
    }

    char *inputPath = argv[1];
    char *outputPath = argv[2];
    int fd1, fd2;
    size_t size;
    char str_buf[buf_size];
    char name1[] = "aaa.fifo";
    char name2[] = "bbb.fifo";

    // Создание именованных каналов
    mknod(name1, S_IFIFO | 0666, 0);
    mknod(name2, S_IFIFO | 0666, 0);

    // Открытие каналов
    fd1 = open(name1, O_WRONLY);
    if (fd1 < 0) {
        perror("open");
        exit(-1);
    }

    // Открытие файла для чтения
    int inputFile = open(inputPath, O_RDONLY);
    if (inputFile < 0) {
        perror("open");
        exit(-1);
    }

    // Чтение данных из файла и запись в канал
    while ((size = read(inputFile, str_buf, buf_size)) > 0) {
        size_t bytes_write = write(fd1, str_buf, size);
        if (bytes_write < 0) {
            perror("write");
            exit(-1);
        }
    }

    // Закрытие канала и файла
    close(fd1);
    close(inputFile);

    // Открытие второго канала для чтения
    fd2 = open(name2, O_RDONLY);
    if (fd2 < 0) {
        perror("open");
        exit(-1);
    }

    // Открытие файла для записи результата
    int outputFile = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (outputFile < 0) {
        perror("open");
        exit(-1);
    }

    // Чтение результата из канала и запись в файл
    while ((size = read(fd2, str_buf, buf_size)) > 0) {
        size_t bytes_write = write(outputFile, str_buf, size);
        if (bytes_write < 0) {
            perror("write");
            exit(-1);
        }
    }

    // Закрытие канала и файла
    close(fd2);
    close(outputFile);

    return 0;
}