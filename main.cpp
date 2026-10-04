#include <iostream>
#include <string>
#include <vector>

#include "AudioConverter.h"

namespace {

void usage(const char* prog) {
    std::cout << "Usage : " << prog << " [options] fichier [fichier...]\n"
              << "        " << prog << "            (mode interactif)\n\n"
              << "Options :\n"
              << "  -f, --format FMT     format de sortie : ";
    for (const auto& f : AudioConverter::supportedFormats()) std::cout << f << ' ';
    std::cout << "\n  -o, --output FICHIER fichier de sortie (un seul fichier d'entree)\n"
              << "  -b, --bitrate DEBIT  ex. 192k (formats avec perte)\n"
              << "  -r, --rate HZ        frequence d'echantillonnage, ex. 44100\n"
              << "  -c, --channels N     1 = mono, 2 = stereo\n"
              << "  -y, --overwrite      ecraser les fichiers existants\n"
              << "  -h, --help           afficher cette aide\n";
}

int interactive() {
    std::string in, fmt;
    std::cout << "Fichier d'entree : ";
    std::getline(std::cin, in);
    std::cout << "Format de sortie (";
    for (const auto& f : AudioConverter::supportedFormats()) std::cout << f << ' ';
    std::cout << ") : ";
    std::getline(std::cin, fmt);
    if (in.size() >= 2 && in.front() == '"' && in.back() == '"') in = in.substr(1, in.size() - 2);

    auto out = AudioConverter::getOutputFilePath(in, fmt);
    auto res = AudioConverter::convert(in, out, fmt);
    if (res.success) std::cout << "Conversion reussie : " << out << "\n";
    else std::cerr << "Echec : " << res.message << "\n";
    return res.success ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 1) return interactive();

    std::string format, output;
    ConversionOptions opts;
    std::vector<std::string> inputs;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&](std::string& dst) {
            if (i + 1 >= argc) { std::cerr << "Valeur manquante pour " << a << "\n"; return false; }
            dst = argv[++i];
            return true;
        };
        std::string v;
        try {
            if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
            else if (a == "-f" || a == "--format") { if (!next(format)) return 2; }
            else if (a == "-o" || a == "--output") { if (!next(output)) return 2; }
            else if (a == "-b" || a == "--bitrate") { if (!next(opts.bitrate)) return 2; }
            else if (a == "-r" || a == "--rate") { if (!next(v)) return 2; opts.sampleRate = std::stoi(v); }
            else if (a == "-c" || a == "--channels") { if (!next(v)) return 2; opts.channels = std::stoi(v); }
            else if (a == "-y" || a == "--overwrite") opts.overwrite = true;
            else if (a.size() > 1 && a[0] == '-') { std::cerr << "Option inconnue : " << a << "\n"; usage(argv[0]); return 2; }
            else inputs.push_back(a);
        } catch (const std::exception&) {
            std::cerr << "Valeur numerique invalide pour " << a << "\n";
            return 2;
        }
    }

    if (inputs.empty()) { usage(argv[0]); return 2; }
    if (format.empty() && !output.empty()) {
        auto dot = output.rfind('.');
        if (dot != std::string::npos) format = output.substr(dot + 1);
    }
    if (!AudioConverter::isSupportedFormat(format)) {
        std::cerr << "Format de sortie manquant ou non supporte (-f)\n";
        return 2;
    }
    if (!output.empty() && inputs.size() > 1) {
        std::cerr << "-o ne peut etre utilise qu'avec un seul fichier d'entree\n";
        return 2;
    }

    int failures = 0;
    for (const auto& in : inputs) {
        const std::string out = output.empty() ? AudioConverter::getOutputFilePath(in, format) : output;
        auto res = AudioConverter::convert(in, out, format, opts);
        if (res.success) std::cout << "OK    " << in << " -> " << out << "\n";
        else { std::cerr << "ECHEC " << in << " : " << res.message << "\n"; ++failures; }
    }
    return failures ? 1 : 0;
}
