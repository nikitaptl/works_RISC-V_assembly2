#include "generateNumber.h"
#include <algorithm>
#include <random>

// Method for generating a vector with unique values in the range [a,b] and with a count of elements
std::vector<int> generateRandomNumbers(int a, int b, int count) {
    if (count > b - a + 1) {
        std::cerr << "Error: Count of numbers to generate exceeds the range." << std::endl;
        return {};
    }

    // Create a vector to store unique numbers
    std::vector<int> numbers;

    // Fill the vector with numbers from a to b
    for (int i = a; i <= b; ++i) {
        numbers.push_back(i);
    }

    // Use random shuffling of the vector
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(numbers.begin(), numbers.end(), g);

    // Return the first count numbers from the shuffled vector
    return std::vector<int>(numbers.begin(), numbers.begin() + count);
}

// Method for generating a random value in the range [a,b]
int generateRandomNumber(int min, int max) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}