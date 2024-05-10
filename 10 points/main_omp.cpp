#include <map>
#include <vector>
#include <cctype>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <omp.h>

#include "generateNumber.h"

#define RED_TEXT "\033[31m"
#define YELLOW_TEXT "\033[33m"
#define RESET_TEXT "\033[0m"
#define VIOLET_TEXT "\x1B[35m"
#define BLUE_TEXT "\x1B[36m"

std::map<char, int> encryption;
std::string original_text;

struct Package {
    int number_priority;
    int start;
    int count;
    std::string result;
};

void encryptString(Package *pack) {
    std::string str_to_encrypt = "";

#pragma omp critical
    {
        for (int i = 0; i < pack->count; i++) {
            str_to_encrypt += toupper(original_text[pack->start + i]);
        }
    }

    sleep(generateRandomNumber(1, 7));

    for (auto el: str_to_encrypt) {
        std::string encrypted_part = std::to_string(encryption[el]);
        pack->result += encrypted_part;
    }

#pragma omp critical
    {
        std::cout << BLUE_TEXT << "[encryption]" << RESET_TEXT << " Шифратор " << pack->number_priority
                  << " закодировал фрагмент " << YELLOW_TEXT << str_to_encrypt << RESET_TEXT << " в " << pack->result
                  << std::endl;
    }
}

void createEncryption() {
    auto encryption_vec = generateRandomNumbers(100, 126, 26);
    for (char i = 'A'; i <= 'Z'; i++) {
        encryption[i] = encryption_vec[i - 65];
    }
}

void incorrectConsoleInput() {
    std::cout << RED_TEXT << "[error] " << RESET_TEXT
              << "Первым параметром ввода с консоли должен быть соответствующий ключ: \n";
}

void incorrectInputFile() {
    std::cout << RED_TEXT << "[error] " << RESET_TEXT
              << "Не удалось открыть файл по ввёденному пути.";
}

void incorrectOutputFile() {
    std::cout << RED_TEXT << "[error] " << RESET_TEXT
              << "Не удалось записать результат в файл по введённому пути.";
}

int main(int argc, char *argv[]) {
    system("chcp 65001");
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
            buffer << inputFile.rdbuf();
            original_text = buffer.str();
            inputFile.close();
        }
    } else {
        std::cout << "Введите текст, который необходимо закодировать: ";
        std::getline(std::cin, original_text);
    }

    // Метод, создающий словарь с кодами шифрования
    createEncryption();
    std::cout << "Исходный текст: " << original_text << std::endl;

    for (char ch: original_text) {
        if (!isalpha(ch)) {
            std::cout << RED_TEXT << "[error] " << RESET_TEXT
                      << "В введённом тексте есть символы, не являющиеся буквами латинского алфавита.";
            return 0;
        }
    }

    int number_separator;
    if (original_text.length() <= 5) {
        number_separator = 2;
    } else {
        number_separator = generateRandomNumber(2, original_text.length() / 2);
    }

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

#pragma omp parallel
    {
        #pragma omp for
        for (int j = 0; j < packages.size(); j++) {
            encryptString(packages[j]);
        }
    }

    std::string result = "";
    std::cout << VIOLET_TEXT << "[result]" << RESET_TEXT << " Закодированный текст: ";
    for (auto package: packages) {
        std::cout << package->result;
        result += package->result;
    }
    std::cout << std::endl;

    if (path_output == "NULL") {
        std::cout << "Введите имя файла, в который необходимо записать результат: ";
        std::getline(std::cin, path_output);
    }
    std::ofstream outputFile(path_output);
    if (!outputFile.is_open()) {
        incorrectOutputFile();
        return 1;
    }

    outputFile << result;
    outputFile.close();

    std::cout << "Результат успешно записан в файл!";
    return 0;
}