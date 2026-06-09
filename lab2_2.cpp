#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <ncurses.h>

struct WAVHeader {
    char riff[4];
    int chunkSize;
    char wave[4];
    char fmt[4];
    int fmtSize;
    short format;
    short channels;
    int sampleRate;
    int byteRate;
    short align;
    short bits;
};


struct Tablo {
    const char* label;
    int hz;
};

Tablo freqTable[] = {
    {"50", 50},
    {"100", 100},
    {"200", 200},
    {"500", 500},
    {"1K", 1000},
    {"2K", 2000},
    {"5K", 5000},
    {"10K", 10000},
    {"20K", 20000}
};

WAVHeader wav;
std::vector<short> ampl;
int dataSize = 0;

double furryParty(const std::vector<short>& buf, int freq, int rate) 
{
    double re = 0;
    double im = 0;
    int n = buf.size();

    for (int i = 0; i < n; i++) {
        double t = 2.0 * M_PI * freq * i / rate;
        re += buf[i] * cos(t);
        im -= buf[i] * sin(t);
    }

    return sqrt(re * re + im * im) / n;
}

std::vector<double> makeHistogram(int sec)
{
    std::vector<double> out;

    int begin = sec * wav.sampleRate * wav.channels;
    int len = wav.sampleRate * wav.channels;

    if (begin + len >= ampl.size())
        return out;
    std::vector<short> block(ampl.begin() + begin, ampl.begin() + begin + len);

    double mx = 0;

    for (int i = 0; i < 9; i++) {
        double v = furryParty(block, freqTable[i].hz, wav.sampleRate);
        out.push_back(v);
        if (v > mx)
            mx = v;
    }
    if (mx > 0)
        for (double& x : out)
            x /= mx;

    return out;
}

bool readWavnik(const char* path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
        return false;

    char riff[4];
    f.read(riff, 4);


    int chunkSize;
    f.read((char*)&chunkSize, 4);

    char wave[4];
    f.read(wave, 4);

    char chunkId[4];
    int chunkSize2;

    short audioFormat = 0;
    short channels = 0;
    int sampleRate = 0;
    int byteRate = 0;
    short blockAlign = 0;
    short bitsPerSample = 0;

    int foundDataSize = 0;

    while (f.read(chunkId, 4)) {
        f.read((char*)&chunkSize2, 4);

        if (chunkId[0] == 'f' && chunkId[1] == 'm' && chunkId[2] == 't') {
            f.read((char*)&audioFormat, 2);
            f.read((char*)&channels, 2);
            f.read((char*)&sampleRate, 4);
            f.read((char*)&byteRate, 4);
            f.read((char*)&blockAlign, 2);
            f.read((char*)&bitsPerSample, 2);
        }
        else if (chunkId[0] == 'd' && chunkId[1] == 'a' && chunkId[2] == 't' && chunkId[3] == 'a') {
            foundDataSize = chunkSize2;
            ampl.resize(foundDataSize / sizeof(short));
            f.read((char*)ampl.data(), foundDataSize);
            break;
        }
        else {
            f.seekg(chunkSize2, std::ios::cur);
        }
    }

    wav.channels = channels;
    wav.sampleRate = sampleRate;
    wav.byteRate = byteRate;
    wav.bits = bitsPerSample;

    dataSize = foundDataSize;

    return (channels > 0 && sampleRate > 0 && foundDataSize > 0);
}

void render(const std::vector<double>& vals, int sec, int total)
{
    clear();
    box(stdscr, 0, 0);
    mvprintw(5, 3, "Channels: %d", wav.channels);
    mvprintw(6, 3, "Sample rate: %d Hz", wav.sampleRate);
    mvprintw(7, 3, "Byte rate: %d", wav.byteRate);
    mvprintw(9, 3, "Bits per sample: %d", wav.bits);
    mvprintw(10, 3, "Data size: %d bytes", dataSize);

    int totalSec = total;

    mvprintw(12, 3, "Time: %d / %d sec", sec, totalSec);

    int floorY = 30;

    for (int y = 15; y <= floorY; y++)
        mvprintw(y, 5, "|");

    for (int x = 5; x <= 82; x++)
        mvprintw(floorY, x, "-");

    for (int i = 0; i < vals.size(); i++) {
        int h = vals[i] * 15;
        int x = 10 + i * 8;

        for (int y = 0; y < h; y++)
            mvprintw(floorY - y, x, "###");

        mvprintw(floorY + 2, x, "%s", freqTable[i].label);
        mvprintw(floorY - h - 1, x, "%.2f", vals[i]);
    }

    refresh();
}

int main(int argc, char** argv)
{
    if (!readWavnik(argv[1])) {
        std::cout << "err\n";
        return 1;
    }

    int totalSec = ampl.size() / wav.sampleRate / wav.channels;

    initscr();
    noecho();
    keypad(stdscr, true);
    curs_set(0);

    int pos = 0;

    while (true) {
        auto bars = makeHistogram(pos);
        render(bars, pos, totalSec);

        int key = getch();

        if (key == 'q')
            break;

        if (key == KEY_RIGHT && pos < totalSec - 1)
            pos++;

        if (key == KEY_LEFT && pos > 0)
            pos--;
    }

    endwin();
    return 0;
}