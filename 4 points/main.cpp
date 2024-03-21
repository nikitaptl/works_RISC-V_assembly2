#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

const int buf_size = 5000;

int* function(char *str) {
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

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Number of input arguments is incorrect!\n");
        exit(-1);
    }
    char* inputPath = argv[1];
    char* outputPath = argv[2];

    int fd1[2], fd2[2], result1, result2;
    size_t size;
    char str_buf[buf_size];

    if (pipe(fd1) < 0) {
        printf("Can\'t open pipe\n");
        exit(-1);
    }
    if (pipe(fd2) < 0) {
        printf("Can\'t open pipe\n");
        exit(-1);
    }

    result1 = fork();
    if (result1 < 0) {
        printf("Can\'t fork process3\n");
        exit(-1);
    } else if (result1 > 0) {
        result2 = fork();
        if (result2 < 0) {
            printf("Can\'t fork process2\n");
            exit(-1);
        }
        // Parent process - Process 1
        else if (result2 > 0) {
            if (close(fd1[0]) < 0) {
                printf("Process1: Can\'t close reading side of pipe\n");
                exit(-1);
            }

            // Открываем файл для чтения
            int fileDecriptor = open(inputPath, O_RDONLY);
            if (fileDecriptor == -1) {
                printf("Process1: Can't open file on specified input path!\n");
                exit(-1);
            }
            ssize_t bytes_read = read(fileDecriptor, str_buf, buf_size);
            if (bytes_read == -1) {
                close(fileDecriptor);
                printf("Process1: Can't read data from file!\n");
                exit(-1);
            }
            if (close(fileDecriptor) == -1) {
                printf("Process1: Can't close file on specified input path");
                exit(-1);
            }

            size = write(fd1[1], str_buf, bytes_read);
            if (size != bytes_read) {
                printf("Process1: Can\'t write all string to pipe\n");
                exit(-1);
            }
            if (close(fd1[1]) < 0) {
                printf("Process1: Can\'t close writing side of pipe\n");
                exit(-1);
            }
            printf("Process1 exit\n");
        } else { /* Child process2 - PROCESS_2 */
            if (close(fd1[1]) < 0) {
                printf("Process2: Can\'t close writing side of pipe\n");
                exit(-1);
            }
            size = read(fd1[0], str_buf, buf_size);
            if (size < 0) {
                printf("Process2: Can\'t read string from pipe\n");
                exit(-1);
            }

            if (close(fd2[0]) < 0) {
                printf("Process2: Can\'t close reading side of pipe\n");
                exit(-1);
            }
            int* arr = function(str_buf);
            sprintf(str_buf, "Number of digits = %d\nNumber of letters = %d", arr[0], arr[1]);
            size = write(fd2[1], str_buf, strlen(str_buf));
            if (size != strlen(str_buf)) {
                printf("Process2: Can\'t write all string to pipe\n");
                exit(-1);
            }
            if (close(fd2[1]) < 0) {
                printf("Process2: Can\'t close writing side of pipe\n");
                exit(-1);
            }

            printf("Process2 exit\n");
        }

    } else { /* Child process -  PROCESS_3 */
        if (close(fd2[1]) < 0) {
            printf("Process3: Can\'t close writing side of pipe\n");
            exit(-1);
        }
        int bytes_read = read(fd2[0], str_buf, buf_size);
        if (bytes_read < 0) {
            printf("Process3: Can\'t read all string from pipe\n");
            exit(-1);
        }
        if (close(fd2[0]) < 0) {
            printf("Process3: Can\'t close reading side of pipe\n");
            exit(-1);
        }

        // Открываем файл для записи
        int fileDecriptor = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC);
        if (fileDecriptor == -1) {
            printf("Process3: Can't open file on specified output path!\n");
            exit(-1);
        }
        ssize_t bytes_write = write(fileDecriptor, str_buf, bytes_read);
        if (bytes_write == -1) {
            close(fileDecriptor);
            printf("Process3: Can't write data to file!\n");
            exit(-1);
        }
        if (close(fileDecriptor) == -1) {
            printf("Process3: Can't close file on specified output path");
            exit(-1);
        }

        printf("Process3 exit\n");
    }
    return 0;
}
