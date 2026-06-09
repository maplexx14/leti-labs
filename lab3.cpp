#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <locale.h>
#include <ncurses.h>
#include <string>
#include <vector>

static const int BAND_COUNT = 9;
static const double BANDS_HZ[BAND_COUNT] = {
    50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0, 20000.0
};

struct WavInfo {
    uint16_t audioFormat = 0;
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint32_t byteRate = 0;
    uint16_t blockAlign = 0;
    uint16_t bitsPerSample = 0;
    uint32_t dataSize = 0;
    uint32_t riffSize = 0;
    std::string fmtChunkId;
    std::string dataChunkId;
};

static uint16_t read16(const unsigned char* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read32(const unsigned char* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static std::string chunkId(const unsigned char* p) {
    return std::string((const char*)p, 4);
}

static bool readExact(std::ifstream& in, void* buf, size_t n) {
    in.read((char*)buf, (std::streamsize)n);
    return in.gcount() == (std::streamsize)n;
}

static bool loadWav(const std::string& path, WavInfo& info, std::vector<float>& samples) {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in) {
        std::cerr << "Не удалось открыть файл: " << path << "\n";
        return false;
    }

    unsigned char riff[12];
    if (!readExact(in, riff, 12)) {
        std::cerr << "Слишком короткий файл\n";
        return false;
    }
    if (memcmp(riff, "RIFF", 4) != 0 || memcmp(riff + 8, "WAVE", 4) != 0) {
        std::cerr << "Это не WAV (RIFF/WAVE)\n";
        return false;
    }
    info.riffSize = read32(riff + 4);

    bool haveFmt = false;
    bool haveData = false;
    std::vector<unsigned char> rawData;

    while (in) {
        unsigned char ch[8];
        if (!readExact(in, ch, 8)) break;

        std::string id = chunkId(ch);
        uint32_t sz = read32(ch + 4);
        if (id == "fmt ") {
            if (sz < 16) {
                std::cerr << "Некорректный fmt chunk\n";
                return false;
            }
            std::vector<unsigned char> fmt(sz);
            if (!readExact(in, fmt.data(), sz)) {
                std::cerr << "Ошибка чтения fmt\n";
                return false;
            }
            info.fmtChunkId = id;
            info.audioFormat = read16(&fmt[0]);
            info.channels = read16(&fmt[2]);
            info.sampleRate = read32(&fmt[4]);
            info.byteRate = read32(&fmt[8]);
            info.blockAlign = read16(&fmt[12]);
            info.bitsPerSample = read16(&fmt[14]);
            haveFmt = true;
        } else if (id == "data") {
            rawData.resize(sz);
            if (sz > 0 && !readExact(in, rawData.data(), sz)) {
                std::cerr << "Ошибка чтения data\n";
                return false;
            }
            info.dataChunkId = id;
            info.dataSize = sz;
            haveData = true;
            break;
        } else {
            in.seekg(sz, std::ios::cur);
        }
        if (sz % 2 == 1) in.seekg(1, std::ios::cur);
    }

    if (!haveFmt || !haveData) {
        std::cerr << "В файле нет fmt или data\n";
        return false;
    }
    if (info.audioFormat != 1) {
        std::cerr << "Поддерживается только PCM (format=1)\n";
        return false;
    }
    if (info.channels < 1 || info.bitsPerSample != 8 && info.bitsPerSample != 16) {
        std::cerr << "Неподдерживаемый формат сэмплов\n";
        return false;
    }

    size_t frameCount = info.blockAlign > 0 ? rawData.size() / info.blockAlign : 0;
    samples.clear();
    samples.reserve(frameCount);

    for (size_t i = 0; i < frameCount; ++i) {
        size_t off = i * info.blockAlign;
        double sum = 0.0;
        for (uint16_t ch = 0; ch < info.channels; ++ch) {
            size_t pos = off + ch * (info.bitsPerSample / 8);
            if (info.bitsPerSample == 16) {
                int16_t s = (int16_t)read16(&rawData[pos]);
                sum += s / 32768.0;
            } else {
                int8_t s = (int8_t)rawData[pos] - 128;
                sum += s / 128.0;
            }
        }
        samples.push_back((float)(sum / info.channels));
    }
    return true;
}

static double goertzelPower(const float* data, size_t n, double sampleRate, double targetHz) {
    if (n == 0 || sampleRate <= 0.0) return 0.0;
    double k = (double)n * targetHz / sampleRate;
    double w = 2.0 * M_PI * k / (double)n;
    double coeff = 2.0 * cos(w);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        s0 = (double)data[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    double real = s1 - s2 * cos(w);
    double imag = s2 * sin(w);
    return real * real + imag * imag;
}

static void computeBands(const std::vector<float>& samples, uint32_t sampleRate, int second,
                         double out[BAND_COUNT]) {
    for (int i = 0; i < BAND_COUNT; ++i) out[i] = 0.0;

    if (sampleRate == 0) return;
    size_t start = (size_t)second * sampleRate;
    if (start >= samples.size()) return;

    size_t end = start + sampleRate;
    if (end > samples.size()) end = samples.size();
    size_t n = end - start;
    if (n == 0) return;

    const float* window = &samples[start];
    double maxVal = 0.0;
    for (int i = 0; i < BAND_COUNT; ++i) {
        out[i] = goertzelPower(window, n, (double)sampleRate, BANDS_HZ[i]);
        if (out[i] > maxVal) maxVal = out[i];
    }
    if (maxVal > 0.0) {
        for (int i = 0; i < BAND_COUNT; ++i) out[i] /= maxVal;
    }
}

static int totalSeconds(const std::vector<float>& samples, uint32_t sampleRate) {
    if (sampleRate == 0) return 0;
    return (int)(samples.size() / sampleRate);
}

static void drawScreen(const WavInfo& info, const std::vector<float>& samples, int second,
                       const double bands[BAND_COUNT]) {
    erase();
    int maxSec = totalSeconds(samples, info.sampleRate);
    if (maxSec > 0 && second >= maxSec) second = maxSec - 1;
    if (second < 0) second = 0;

    printw("=== Заголовок WAV ===\n");
    printw("RIFF size      : %u\n", info.riffSize);
    printw("Format         : %u (1=PCM)\n", info.audioFormat);
    printw("Channels       : %u\n", info.channels);
    printw("Sample rate    : %u Hz\n", info.sampleRate);
    printw("Byte rate      : %u\n", info.byteRate);
    printw("Block align    : %u\n", info.blockAlign);
    printw("Bits per sample: %u\n", info.bitsPerSample);
    printw("Data size      : %u bytes\n", info.dataSize);
    if (info.sampleRate > 0) {
        double dur = (double)samples.size() / (double)info.sampleRate;
        printw("Duration       : %.2f s (%d full seconds)\n", dur, maxSec);
    }
    printw("\n=== Нормированная гистограмма (окно 1 с) ===\n");
    printw("Интервал: [%d; %d) с   ", second, second + 1);
    if (maxSec == 0) printw("(нет данных)\n");
    else printw("\n");

    const int chartHeight = 10;
    const int barWidth = 4;
    for (int row = chartHeight; row >= 1; --row) {
        double level = (double)row / (double)chartHeight;
        printw("%4.1f|", level);
        for (int i = 0; i < BAND_COUNT; ++i) {
            if (bands[i] + 1e-9 >= level) {
                for (int k = 0; k < barWidth; ++k) addch('#');
            } else {
                for (int k = 0; k < barWidth; ++k) addch(' ');
            }
            addch(' ');
        }
        printw("\n");
    }
    printw("     +");
    for (int i = 0; i < BAND_COUNT; ++i) {
        for (int k = 0; k < barWidth; ++k) addch('-');
        addch(' ');
    }
    printw("\n      ");
    for (int i = 0; i < BAND_COUNT; ++i) {
        char label[8];
        if (BANDS_HZ[i] >= 1000.0)
            std::snprintf(label, sizeof(label), "%dk", (int)(BANDS_HZ[i] / 1000.0));
        else
            std::snprintf(label, sizeof(label), "%d", (int)BANDS_HZ[i]);
        printw("%*s ", barWidth, label);
    }
    printw("\n      ");
    for (int i = 0; i < BAND_COUNT; ++i) printw("%*s ", barWidth, "Hz");
    printw("\n\n");

    printw("Значения: ");
    for (int i = 0; i < BAND_COUNT; ++i) printw("%.3f ", bands[i]);
    printw("\n\n");

    printw("Управление:\n");
    printw("  <- / ->  или  a / d  — сдвиг на 1 с\n");
    printw("  Home / End           — к началу / концу\n");
    printw("  q                    — выход\n");
    refresh();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <файл.wav>\n";
        return 1;
    }

    WavInfo info;
    std::vector<float> samples;
    if (!loadWav(argv[1], info, samples)) return 1;

    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int second = 0;
    double bands[BAND_COUNT];
    bool running = true;

    while (running) {
        int maxSec = totalSeconds(samples, info.sampleRate);
        if (maxSec > 0 && second >= maxSec) second = maxSec - 1;
        if (second < 0) second = 0;

        computeBands(samples, info.sampleRate, second, bands);
        drawScreen(info, samples, second, bands);

        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
            case 'a':
            case 'A':
                second--;
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                second++;
                break;
            case KEY_HOME:
                second = 0;
                break;
            case KEY_END:
                second = maxSec > 0 ? maxSec - 1 : 0;
                break;
            case 'q':
            case 'Q':
            case 27:
                running = false;
                break;
            default:
                break;
        }
    }

    endwin();
    return 0;
}
