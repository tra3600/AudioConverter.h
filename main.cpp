#include <iostream>
#include "AudioConverter.h"

int main() {
    std::string inputFilePath, outputFormat;

    std::cout << "Enter the input file path: ";
    std::getline(std::cin, inputFilePath);

    std::cout << "Enter the output format (mp3, wav, m4a): ";
    std::getline(std::cin, outputFormat);

    std::string outputFilePath = AudioConverter::getOutputFilePath(inputFilePath, outputFormat);

    if (AudioConverter::convert(inputFilePath, outputFilePath, outputFormat)) {
        std::cout << "Conversion successful! Output file: " << outputFilePath << std::endl;
    } else {
        std::cerr << "Conversion failed!" << std::endl;
    }

    return 0;
}