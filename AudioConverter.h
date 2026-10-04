#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <string>
#include <vector>

// Options de conversion (0 / vide = valeur par defaut de FFmpeg).
struct ConversionOptions {
    std::string bitrate;     // ex. "192k" (formats avec perte uniquement)
    int sampleRate = 0;      // ex. 44100
    int channels = 0;        // 1 = mono, 2 = stereo
    bool overwrite = false;  // ecraser le fichier de sortie s'il existe
};

struct ConversionResult {
    bool success = false;
    std::string message;     // erreur ou information
};

class AudioConverter {
public:
    // Formats pris en charge : mp3, wav, m4a, flac, ogg, aac.
    static const std::vector<std::string>& supportedFormats();
    static bool isSupportedFormat(const std::string& format);

    // Normalise un format saisi par l'utilisateur (".MP3" -> "mp3").
    static std::string normalizeFormat(const std::string& format);

    // Remplace l'extension de l'entree par celle du format de sortie.
    static std::string getOutputFilePath(const std::string& inputFilePath,
                                         const std::string& outputFormat);

    // Verifie que FFmpeg est installe et executable.
    static bool isFFmpegAvailable();

    // Convertit un fichier. Les arguments sont passes directement a FFmpeg
    // (sans shell) : les espaces et caracteres speciaux sont sans danger.
    static ConversionResult convert(const std::string& inputFilePath,
                                    const std::string& outputFilePath,
                                    const std::string& outputFormat,
                                    const ConversionOptions& options = {});

    // Construit la liste d'arguments FFmpeg (exposee pour les tests).
    static std::vector<std::string> buildArguments(const std::string& inputFilePath,
                                                   const std::string& outputFilePath,
                                                   const std::string& outputFormat,
                                                   const ConversionOptions& options);
};

#endif // AUDIOCONVERTER_H
