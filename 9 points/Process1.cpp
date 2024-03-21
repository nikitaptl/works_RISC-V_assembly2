#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

const int buf_size = 100; // Размер буфера для чтения из файла и записи в канал

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        exit(-1);
    }

    char *inputPath = argv[1];
    char *outputPath = argv[2];
    int fd, outputFile;
    size_t size;
    char str_buf[buf_size];
    char name1[] = "aaa.fifo";
    char name2[] = "bbb.fifo";

    // Создание именованных каналов
    mknod(name1, S_IFIFO | 0666, 0);
    mknod(name2, S_IFIFO | 0666, 0);

    // Открытие каналов
    fd = open(name1, O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(-1);
    }

    // Открытие файла для чтения
    int inputFile = open(inputPath, O_RDONLY);
    if (inputFile < 0) {
        perror("open");
        exit(-1);
    }

    // Открытие файла для записи
    outputFile = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (outputFile < 0) {
        perror("open");
        exit(-1);
    }

    // Чтение данных из файла и запись в канал
    while ((size = read(inputFile, str_buf, buf_size)) > 0) {
        size_t written = write(fd, str_buf, size);
        if (written < 0) {
            perror("write");
            exit(-1);
        }

        // Ожидание ответа от второго процесса
        int response_fd = open(name2, O_RDONLY);
        if (response_fd < 0) {
            perror("open");
            exit(-1);
        }
        char response_buf[buf_size];
        ssize_t response_size = read(response_fd, response_buf, buf_size);
        if (response_size < 0) {
            perror("read");
            exit(-1);
        }

        // Запись результата обработки в файл
        written = write(outputFile, response_buf, response_size);
        if (written < 0) {
            perror("write");
            exit(-1);
        }
        close(response_fd);
    }

    // Закрытие канала и файлов
    close(fd);
    close(inputFile);
    close(outputFile);

    return 0;
}