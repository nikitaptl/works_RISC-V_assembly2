#include <unistd.h>
#include <pthread.h>
#include <random>
#include <iostream>
#include <semaphore.h>

#define VIOLET_TEXT "\x1B[35m"
#define RESET_TEXT "\x1B[0m"
#define BLUE_TEXT "\x1B[36m"

const size_t bufer_size = 20;
int bufer[bufer_size];
int count = 0;
int result_prom = 0;
int result = -1; // Результат для сравнения. Когда обычное суммирование закончится, в него будет внесен результат обычного суммирования.

pthread_mutex_t mutex;
sem_t semaphore;

int generateRandomNumber(int min, int max) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

void *informationSource(void *param) {
    int sNum = *((int *) param);

    int number = generateRandomNumber(1, 20); // Сгенерированное источником информации число
    sleep(generateRandomNumber(1, 7)); // Задержка до поступления в буфер

    pthread_mutex_lock(&mutex);  // Поступление числа в буфер
    bufer[count] = number;
    std::cout << BLUE_TEXT << "[generating]" << RESET_TEXT << " Источник информации " << sNum
              << " сгенерировал число: buffer[" << count << "] = " << bufer[count] << std::endl;

    count++;
    if (count >= 2) { // Разбудить трекер для суммирования
        sem_post(&semaphore);
    }
    result_prom += number;
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

void *summator(void *arg) {
    sleep(generateRandomNumber(3, 6)); // Время суммирования
    pthread_mutex_lock(&mutex);
    int num1 = bufer[count - 2];
    int num2 = bufer[count - 1];
    count -= 1;
    bufer[count - 1] = num1 + num2;
    std::cout << VIOLET_TEXT << "[sum]" << RESET_TEXT << " Просуммированы " << num1 << " и " << num2 << ", получено "
              << num1 + num2 << std::endl;
    if (bufer[0] == result) {
        sem_post(&semaphore);
    }
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

void *tracking(void *arg) {
    while (true) {
        sem_wait(&semaphore);
        if (bufer[0] == result) {
            break;
        }
        pthread_mutex_lock(&mutex);
        pthread_t thread_sum;
        pthread_create(&thread_sum, NULL, summator, (void *) (NULL));
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

int main() {
    system("chcp 65001");

    sem_init(&semaphore, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threadS[20];
    int informationSources[20];
    for (int i = 0; i < 20; i++) {
        informationSources[i] = i + 1;
        pthread_create(&threadS[i], NULL, informationSource, (void *) (informationSources + i));
    }

    pthread_t tracker;
    pthread_create(&tracker, NULL, tracking, (void *) (NULL));

    for (auto i: threadS) {
        pthread_join(i, NULL);
    }
    result = result_prom;
    pthread_join(tracker, NULL);

    std::cout << "a) Результат, полученный обычным суммированием: " << result << std::endl;
    std::cout << "b) Результат, полученный программой: " << bufer[0] << std::endl;
    if (result == bufer[0]) {
        std::cout << "Программа правильно вычислила сумму!";
    }
    return 0;
}