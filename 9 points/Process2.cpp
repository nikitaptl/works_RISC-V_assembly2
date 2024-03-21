#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

const int buf_size = 100; // Размер буфера для чтения из канала и записи в канал

int *function(char *str) {
    // Подсчет числа цифр и букв
    static int result[2] = {0, 0};
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i])) {
            result[0]++;
        } else if (isalpha(str[i])) {
            result[1]++;
        }
    }
    return result;
}

int main() {
    int fd1, fd2;
    size_t size;
    char str_buf[buf_size];
    char name1[] = "aaa.fifo";
    char name2[] = "bbb.fifo";

    // Открытие каналов
    fd1 = open(name1, O_RDONLY);
    if (fd1 < 0) {
        perror("open");
        exit(-1);
    }

    fd2 = open(name2, O_WRONLY);
    if (fd2 < 0) {
        perror("open");
        exit(-1);
    }

    // Чтение данных из канала и обработка
    while ((size = read(fd1, str_buf, buf_size)) > 0) {
        // Обработка данных
        int *result = function(str_buf);

        // Передача результата обработки обратно в первый процесс через второй канал
        size_t written = write(fd2, result, 2 * sizeof(int));
        if (written < 0) {
            perror("write");
            exit(-1);
        }
    }

    // Закрытие каналов
    close(fd1);
    close(fd2);

    return 0;
}