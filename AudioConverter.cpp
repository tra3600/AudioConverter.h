#include "AudioConverter.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {

// Execute `args` (args[0] = programme) sans passer par un shell.
// Retourne le code de sortie, ou -1 si le lancement a echoue.
int runProcess(const std::vector<std::string>& args, bool quiet) {
    std::vector<char*> argv;
    for (const auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);

#ifdef _WIN32
    (void)quiet;
    return static_cast<int>(_spawnvp(_P_WAIT, argv[0], argv.data()));
#else
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        if (quiet) {
            if (!freopen("/dev/null", "w", stdout)) _exit(127);
            if (!freopen("/dev/null", "w", stderr)) _exit(127);
        }
        execvp(argv[0], argv.data());
        _exit(127);  // programme introuvable
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

}  // namespace

const std::vector<std::string>& AudioConverter::supportedFormats() {
    static const std::vector<std::string> formats = {"mp3", "wav", "m4a", "flac", "ogg", "aac"};
    return formats;
}

std::string AudioConverter::normalizeFormat(const std::string& format) {
    std::string f = format;
    f.erase(f.begin(), std::find_if(f.begin(), f.end(), [](unsigned char c) { return !std::isspace(c) && c != '.'; }));
    f.erase(std::find_if(f.rbegin(), f.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), f.end());
    std::transform(f.begin(), f.end(), f.begin(), [](unsigned char c) { return std::tolower(c); });
    return f;
}

bool AudioConverter::isSupportedFormat(const std::string& format) {
    const auto& fmts = supportedFormats();
    return std::find(fmts.begin(), fmts.end(), normalizeFormat(format)) != fmts.end();
}

std::string AudioConverter::getOutputFilePath(const std::string& inputFilePath,
                                              const std::string& outputFormat) {
    fs::path p(inputFilePath);
    p.replace_extension("." + normalizeFormat(outputFormat));
    return p.string();
}

bool AudioConverter::isFFmpegAvailable() {
    return runProcess({"ffmpeg", "-version"}, true) == 0;
}

std::vector<std::string> AudioConverter::buildArguments(const std::string& inputFilePath,
                                                        const std::string& outputFilePath,
                                                        const std::string& outputFormat,
                                                        const ConversionOptions& options) {
    const std::string fmt = normalizeFormat(outputFormat);
    std::vector<std::string> args = {"ffmpeg", "-hide_banner", "-loglevel", "error",
                                     options.overwrite ? "-y" : "-n",
                                     "-i", inputFilePath,
                                     "-vn"};  // ignore pochettes / pistes video

    // Codec et conteneur explicites selon le format.
    if (fmt == "mp3") args.insert(args.end(), {"-codec:a", "libmp3lame", "-f", "mp3"});
    else if (fmt == "wav") args.insert(args.end(), {"-codec:a", "pcm_s16le", "-f", "wav"});
    else if (fmt == "m4a") args.insert(args.end(), {"-codec:a", "aac", "-f", "ipod"});
    else if (fmt == "aac") args.insert(args.end(), {"-codec:a", "aac", "-f", "adts"});
    else if (fmt == "flac") args.insert(args.end(), {"-codec:a", "flac", "-f", "flac"});
    else if (fmt == "ogg") args.insert(args.end(), {"-codec:a", "libvorbis", "-f", "ogg"});

    const bool lossless = (fmt == "wav" || fmt == "flac");
    if (!options.bitrate.empty() && !lossless) args.insert(args.end(), {"-b:a", options.bitrate});
    if (options.sampleRate > 0) args.insert(args.end(), {"-ar", std::to_string(options.sampleRate)});
    if (options.channels > 0) args.insert(args.end(), {"-ac", std::to_string(options.channels)});

    args.push_back(outputFilePath);
    return args;
}

ConversionResult AudioConverter::convert(const std::string& inputFilePath,
                                         const std::string& outputFilePath,
                                         const std::string& outputFormat,
                                         const ConversionOptions& options) {
    if (!isSupportedFormat(outputFormat))
        return {false, "Format de sortie non supporte : '" + outputFormat + "'"};
    if (inputFilePath.empty()) return {false, "Aucun fichier d'entree"};
    if (!fs::is_regular_file(inputFilePath))
        return {false, "Fichier d'entree introuvable : " + inputFilePath};
    std::error_code ec;
    if (fs::exists(outputFilePath, ec) && fs::equivalent(inputFilePath, outputFilePath, ec))
        return {false, "Les fichiers d'entree et de sortie sont identiques"};
    if (fs::exists(outputFilePath, ec) && !options.overwrite)
        return {false, "Le fichier de sortie existe deja (utiliser -y pour ecraser) : " + outputFilePath};
    if (!isFFmpegAvailable()) return {false, "FFmpeg est introuvable : installez-le et ajoutez-le au PATH"};

    const int code = runProcess(buildArguments(inputFilePath, outputFilePath, outputFormat, options), false);
    if (code != 0) {
        std::remove(outputFilePath.c_str());  // pas de fichier partiel
        return {false, "FFmpeg a echoue (code " + std::to_string(code) + ")"};
    }
    return {true, outputFilePath};
}
