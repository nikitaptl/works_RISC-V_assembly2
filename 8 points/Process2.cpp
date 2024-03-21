#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

const int buf_size = 5000;

int *function(char *str) {
    // Подсчёт длины строки
    int num = strlen(str);
    int numDigit = 0;
    int numAlpha = 0;
    for (int i = 0; i < num; i++) {
        if (isalpha(str[i])) {
            numAlpha++;
        } else if (isdigit(str[i])) {
            numDigit++;
        }
    }
    static int array[2];
    array[0] = numDigit;
    array[1] = numAlpha;
    return array;
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

    // Чтение данных из первого канала, обработка и отправка результата во второй канал
    while ((size = read(fd1, str_buf, buf_size)) > 0) {
        int *arr = function(str_buf);
        sprintf(str_buf, "Number of digits = %d\nNumber of letters = %d", arr[0], arr[1]);
        size_t written = write(fd2, str_buf, strlen(str_buf));
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