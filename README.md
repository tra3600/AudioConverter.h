# AudioConverter.h

Convertisseur audio en C++17 (MP3, WAV, M4A, FLAC, OGG, AAC) basé sur FFmpeg.
Toutes les conversions entre ces formats sont possibles (MP3 ⇄ WAV, M4A ⇄ WAV, M4A ⇄ MP3, …).

## Prérequis

- [FFmpeg](https://ffmpeg.org/) installé et accessible dans le `PATH`
- Un compilateur C++17 (g++, clang, MSVC) — CMake optionnel

## Compilation

```bash
g++ -std=c++17 -o audio_converter main.cpp AudioConverter.cpp
# ou
cmake -B build && cmake --build build
```

## Utilisation

```bash
./audio_converter -f mp3 chanson.wav                # -> chanson.mp3
./audio_converter -f m4a -b 192k *.wav              # plusieurs fichiers, 192 kb/s
./audio_converter -f wav -r 44100 -c 2 in.m4a       # 44,1 kHz stéréo
./audio_converter -y -o sortie.flac entree.mp3      # écrase, format déduit de -o
./audio_converter                                   # mode interactif
```

| Option | Description |
|---|---|
| `-f, --format` | format de sortie (mp3, wav, m4a, flac, ogg, aac) |
| `-o, --output` | fichier de sortie (un seul fichier d'entrée) |
| `-b, --bitrate` | débit, ex. `192k` (formats avec perte) |
| `-r, --rate` | fréquence d'échantillonnage en Hz |
| `-c, --channels` | 1 = mono, 2 = stéréo |
| `-y, --overwrite` | écraser les fichiers existants |

Code de retour : `0` succès, `1` échec de conversion, `2` erreur d'arguments.

## API

```cpp
#include "AudioConverter.h"

ConversionOptions opts;
opts.bitrate = "192k";
auto out = AudioConverter::getOutputFilePath("in.wav", "mp3");
auto res = AudioConverter::convert("in.wav", out, "mp3", opts);
if (!res.success) std::cerr << res.message << "\n";
```

## Notes de conception

- FFmpeg est lancé directement (`fork`/`execvp`, `_spawnvp` sous Windows), **sans shell** : les espaces et caractères spéciaux dans les noms de fichiers sont sans danger (pas d'injection de commande).
- Codec et conteneur explicites par format ; les pochettes/pistes vidéo sont ignorées.
- Vérifications préalables : FFmpeg présent, entrée existante, sortie différente de l'entrée, pas d'écrasement sans `-y`. Un fichier de sortie partiel est supprimé en cas d'échec.
