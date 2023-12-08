#include <iostream>
#include <iomanip>
#include <limits>
#include <ctime>
#include <pthread.h>

unsigned long long arrSize;
unsigned int threadNumber;

struct Package {
    double *A;
    double *B;
    int threadNum;
    long double sum;
};

void *func(void *package) {
    Package *pack = (Package *) package;

    for (unsigned int i = pack->threadNum; i < arrSize; i += threadNumber) {
        pack->sum += pack->A[i] * pack->B[i];
    }
    return nullptr;
}

int main() {
    system("chcp 65001");
    long double result = 0.0;

    std::cout << "Введите число элементов в векторах: ";
    std::cin >> arrSize;

    std::cout << "Введите число потоков: ";
    std::cin >> threadNumber;

    double *A = new double[arrSize];
    double *B = new double[arrSize];

    for (int i = 0; i < arrSize; i++) {
        A[i] = double(i + 1);
    }
    for (int i = 0; i < arrSize; i++) {
        B[i] = double(arrSize - i);
    }

    pthread_t thread[threadNumber];
    Package pack[threadNumber];

    clock_t start_time = clock();

    for (int i = 0; i < threadNumber; i++) {
        pack[i].A = A;
        pack[i].B = B;
        pack[i].threadNum = i;
        pack[i].sum = 0.0;
        pthread_create(&thread[i], nullptr, func, (void *) &pack[i]);
    }

    clock_t thread_started_time = clock();

    for (int i = 0; i < threadNumber; i++) {
        pthread_join(thread[i], nullptr);
        result += pack[i].sum;
    }

    clock_t end_time = clock();

    std::cout << "Сумма квадратов = " << result << "\n";
    std::cout << "Сумма квадратов = " <<
              std::setprecision(20) << result << "\n";
    std::cout << "Время счета и сборки = " << end_time - start_time << "\n";
};