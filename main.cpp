#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

unsigned long long Factorial(unsigned long long n) {
    if (n == 0 || n == 1) {
        return 1;
    }
    return n * Factorial(n - 1);
}

unsigned long long Fibonacci(unsigned long long n) {
    if (n == 0 || n == 1) {
        return n;
    }
    return Fibonacci(n - 1) + Fibonacci(n - 2);
}

int main() {
    // Получение вхрдного значения
    unsigned long long inputNumber;
    std::cin >> inputNumber;

    // Создание процесса-ребенка для вычисления факториала
    pid_t chPid = fork();

    if (chPid == -1) {
        std::cerr << "Ошибка при создании процесса для факториала" << std::endl;
        return 1;
    } else if (chPid == 0) {
        // Выполнение действий ребёнка
        std::cout << "Ребёнок: PID=" << getpid() << ", PPID=" << getppid() << ", CHILD = " << chPid << ", Факториал("
                  << inputNumber << ") = " << Factorial(inputNumber) << std::endl;

        // Вывод информации о содержимом текущего каталога
        execlp("ls", "ls", "-l", nullptr);
        return 0;
    } else {
        // Родительский процесс
        int status;
        sleep(1);

        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGFPE) {
            std::cerr << "Произошло переполнение при вычислении факториала." << std::endl;
        } else {
            // Вычисление числа Фибоначчи
            unsigned long long fibonacciResult = Fibonacci(inputNumber);
            std::cout << "Родитель: PID = " << getpid() << ", PPID = " << getppid() << ", CHILD = " << chPid
                      << ", Фибоначчи(" << inputNumber << ") = " << fibonacciResult << std::endl;
        }
        std::cout << "Родительский процесс завершен." << std::endl;
    }
    execlp("ls", "ls", "-l", nullptr);
    return 0;
}
