#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

unsigned int binaryNumber(unsigned int n) {
    int res = 0;
    while(n > 0) {
        res++;
        n /= 2;
    }
    return res;
}

int main() {
    pid_t pid;
    int number;

    printf("Введите число, которое необходимо передать: ");
    scanf("%d", &number);

    printf("Введите pid приёмника: ");
    scanf("%d", &pid);

    for (int i = 0; i < binaryNumber(number); i++) {
        if (number & (1 << i)) {
            printf("Отправляю бит 1\n");
            kill(pid, SIGUSR1); // Отправляем бит 1
        } else {
            printf("Отправляю бит 0\n");
            kill(pid, SIGUSR2); // Отправляем бит 0
        }
        sleep(1);
    }

    printf("Отправка числа завершена\n");
    kill(pid, SIGIO); // Отправляем сигнал о том, что передача битов завершена
    return 0;
}