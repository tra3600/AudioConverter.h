#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <string>

class AudioConverter {
public:
    static bool convert(const std::string& inputFilePath, const std::string& outputFilePath, const std::string& outputFormat);

private:
    static std::string getOutputFilePath(const std::string& inputFilePath, const std::string& outputFormat);
};

#endif // AUDIOCONVERTER_H
