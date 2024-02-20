#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

unsigned long long calculateFactorial(unsigned long long n) {
    if (n == 0 || n == 1) {
        return 1;
    } else {
        return n * calculateFactorial(n - 1);
    }
}

unsigned long long calculateFibonacci(unsigned long long n) {
    if (n == 0 || n == 1) {
        return n;
    } else {
        return calculateFibonacci(n - 1) + calculateFibonacci(n - 2);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Использование: " << argv[0] << " <число>" << endl;
        return 1;
    }

    // Получение аргумента из командной строки
    unsigned long long inputNumber = strtoull(argv[1], nullptr, 10);

    // Создание процесса-ребенка для вычисления факториала
    pid_t factorialProcess = fork();

    if (factorialProcess == -1) {
        cerr << "Ошибка при создании процесса для факториала." << endl;
        return 1;
    } else if (factorialProcess == 0) {
        // Код для процесса-ребенка (факториал)
        cout << "Процесс-ребенок (факториал): PID=" << getpid() << ", PPID=" << getppid() << endl;

        unsigned long long result = calculateFactorial(inputNumber);
        cout << "Факториал(" << inputNumber << ") = " << result << endl;

        // Вывод информации о содержимом текущего каталога
        execlp("ls", "ls", "-l", nullptr);
        
        // В случае ошибки execlp
        cerr << "Ошибка при выполнении ls." << endl;
        return 1;
    } else {
        // Код для родительского процесса (число Фибоначчи)
        cout << "Родительский процесс (Фибоначчи): PID=" << getpid() << ", PPID=" << getppid() << endl;

        // Ожидание завершения процесса-ребенка для факториала
        int status;
        waitpid(factorialProcess, &status, 0);

        // Проверка на переполнение
        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGFPE) {
            cerr << "Произошло переполнение при вычислении факториала." << endl;
        } else {
            // Вычисление числа Фибоначчи
            unsigned long long fibonacciResult = calculateFibonacci(inputNumber);
            cout << "Фибоначчи(" << inputNumber << ") = " << fibonacciResult << endl;
        }

        cout << "Родительский процесс завершен." << endl;
        return 0;
    }
}
