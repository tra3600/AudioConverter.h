#include "AudioConverter.h"
#include <iostream>
#include <cstdlib>

bool AudioConverter::convert(const std::string& inputFilePath, const std::string& outputFilePath, const std::string& outputFormat) {
    std::string command = "ffmpeg -i " + inputFilePath + " " + outputFilePath;
    int result = system(command.c_str());
    return result == 0;
}

std::string AudioConverter::getOutputFilePath(const std::string& inputFilePath, const std::string& outputFormat) {
    size_t dotPos = inputFilePath.rfind('.');
    if (dotPos == std::string::npos) {
        return inputFilePath + "." + outputFormat;
    }
    return inputFilePath.substr(0, dotPos) + "." + outputFormat;
}