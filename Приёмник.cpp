#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t number = 0;
volatile sig_atomic_t bit = 0;
bool flag = true;

void sigusr1_handler(int signum) {
    printf("Получен 1\n");
    number |= (1 << bit); // Устанавливаем бит в 1
    bit++;
}

void sigusr2_handler(int signum) {
    printf("Получен 0\n");
    number &= ~(1 << bit); // Устанавливаем бит в 0
    bit++;
}

void sigio_handler(int signum) {
    printf("Получение битов завершено\n");
    flag = false;
}

int main() {
    printf("Мой pid = %d\n", getpid());

    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGIO, sigio_handler);

    while (flag) {
    }
    printf("Число = %d\n", number);

    return 0;
}