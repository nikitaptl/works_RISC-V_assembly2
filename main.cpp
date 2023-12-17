#include <map>
#include <vector>
#include <pthread.h>
#include <cctype>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "generateNumber.h"

#define RED_TEXT "\033[31m"
#define YELLOW_TEXT "\033[33m"
#define RESET_TEXT "\033[0m"
#define VIOLET_TEXT "\x1B[35m"
#define BLUE_TEXT "\x1B[36m"

std::map<char, int> encryption;
std::string original_text;
pthread_mutex_t mutex; // Мьютекс для защиты original_string для чтения

struct Package {
    int number_priority; // Номер участка исходного текста
    int start;
    int count;
    std::string result; // Поле для результата
};

void *encryptString(void *param) {
    auto *pack = (Package *) param;
    std::string str_to_encrypt = "";

    // Блокируем мьютекс для доступа чтения original_text
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < pack->count; i++) {
        str_to_encrypt += toupper(original_text[pack->start + i]);
    }
    pthread_mutex_unlock(&mutex);
    // После того, как прочитали, разблокируем мьютекс

    sleep(generateRandomNumber(1, 7));
    for (auto el: str_to_encrypt) { // Во входной указатель заносятся данные о закодированном тексте1
        std::string encrypted_part = std::to_string(encryption[el]);
        pack->result += encrypted_part;
    }
    std::cout << BLUE_TEXT << "[encryption]" << RESET_TEXT << " Шифратор " << pack->number_priority
              << " закодировал фрагмент " << YELLOW_TEXT << str_to_encrypt << RESET_TEXT << " в " << pack->result
              << std::endl;
    return nullptr;
}

void createEncryption() { // Метод, создающий словарь, включающий в себя
    // коды шифрования (пара 'char' латинский символ - цифра)
    /* Диапозон 100 - 126 был выбран потому, что это минимальный диапозон чисел, позволяющий корректно закодировать текст
    (чтобы ни одна из цифр не включала в себя другую) */
    auto encryption_vec = generateRandomNumbers(100, 126, 26);
    for (char i = 'A'; i <= 'Z'; i++) {
        encryption[i] = encryption_vec[i - 65];
    }
}

// Вызывается, если не были введены корректные ключи
void incorrectConsoleInput() {
    std::cout << RED_TEXT << "[error] " << RESET_TEXT
              << "Первым параметром ввода с консоли должен быть соответствующий ключ: \n";
    std::cout << "0 - В консоль вводится только исходная строка\n";
    std::cout << "1 - В консоль вводятся исходная строка и адрес файла вывода результатов\n";
    std::cout << "2 - В консоль вводятся файл с исходными данными и файл вывода результатов\n";
    std::cout << "За этим ключом идут соответствующие параметры. Попробуйте ещё раз!" << std::endl;
}

// Выводится, если названия файла с входными данными является некорректным
void incorrectInputFile() {
    std::cout << RED_TEXT << "[error] " << RESET_TEXT
              << "Не удалось открыть файл по ввёденному пути.";
}

// Выводится, если название файла для вывода является некорректным
void incorrectOutputFile() {
    std::cout << RED_TEXT << "[error] " << RESET_TEXT
              << "Не удалось записать результат в файл по введённому пути.";
}

int main(int argc, char *argv[]) {
    system("chcp 65001");
    // Если ввод с консоли, то происходит проверка его корректности:
    std::string path_output = "NULL";
    if (argc != 1) {
        int par;
        try {
            par = std::stoi(argv[1]);
            if (par < 0 || par > 2 || argc > 4) {
                incorrectConsoleInput();
                return 0;
            }
        }
        catch (std::exception e) {
            incorrectConsoleInput();
            return 0;
        }
        if (par == 0) {
            original_text = argv[2];
        }
        if (par == 1) {
            original_text = argv[2];
            path_output = argv[3];
        }
        if (par == 2) {
            path_output = argv[3];
            std::ifstream inputFile(argv[2]);
            if (!inputFile.is_open()) {
                incorrectInputFile();
                return 0;
            }

            std::stringstream buffer;
            buffer << inputFile.rdbuf(); // Чтение всего содержимого файла в поток stringstream
            original_text = buffer.str(); // Получение строки из stringstream

            inputFile.close();
        }
    } else {
        std::cout << "Введите текст, который необходимо закодировать: ";
        // Входные данные - строка(текст), который необходимо закодировать
        std::getline(std::cin, original_text);
    }
    pthread_mutex_init(&mutex, nullptr); // Инициируем мьютекст

    // Метод, создающий словарь с кодами шифрования
    createEncryption();
    std::cout << "Исходный текст: " << original_text << std::endl;

    // Проверка на то, все ли символы являются буквами латинского алфавита
    for (char ch: original_text) {
        if (!isalpha(ch)) {
            std::cout << RED_TEXT << "[error] " << RESET_TEXT
                      << "В введённом тексте есть символы, не являющиеся буквами латинского алфавита.";
            return 0;
        }
    }

    // number_separator - число, обозначающее сколько символов будет в каждом из фрагментов исходного текста
    // Минимальное количество = 2. Максимальное - половина размера исходного текста
    int number_separator;
    if (original_text.length() <= 5) {
        number_separator = 2;
    } else {
        number_separator = generateRandomNumber(2, original_text.length() / 2);
    }

    // Создаём вектор указаталей на входные значения для каждого потока. В них кладём фрагменты исходной строки.
    std::vector<Package *> packages;
    int i;
    for (i = 0; i < original_text.length() / number_separator; i++) {
        packages.push_back(new Package{i, i * number_separator, number_separator, ""});
    }
    if (original_text.length() % number_separator != 0) {
        packages.push_back(
                new Package{i, i * number_separator, static_cast<int>((original_text.length() - i * number_separator)),
                            ""});
    }

    // Создаём потоки
    pthread_t threads[packages.size()];
    for (int j = 0; j < packages.size(); j++) {
        pthread_create(&threads[j], nullptr, encryptString, (void *) (packages[j]));
    }
    // Ждём, пока каждый из потоков выполнится
    for (int j = 0; j < packages.size(); j++) {
        pthread_join(threads[j], nullptr);
    }

    // Выводим результат на экран
    std::string result = "";
    std::cout << VIOLET_TEXT << "[result]" << RESET_TEXT << " Закодированный текст: ";
    for (auto package: packages) {
        std::cout << package->result;
        result += package->result;
    }
    std::cout << std::endl;

    // Если пользователь не ввёл имя выходного файла в командной строке, просим его в консоли:
    if (path_output == "NULL") {
        std::cout << "Введите имя файла, в который необходимо записать результат: ";
        std::getline(std::cin, path_output);
    }
    std::ofstream outputFile(path_output);
    if (!outputFile.is_open()) {
        incorrectOutputFile();
        return 1;
    }
    // Запись строки в файл
    outputFile << result;
    outputFile.close();

    std::cout << "Результат успешно записан в файл!";
    return 0;
}