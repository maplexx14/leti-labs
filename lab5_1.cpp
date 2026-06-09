#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <vector>
#include <locale.h>

//  ДИСПИСК СДЕЛАЙ ЛАБОРАТРНУЮ ЧТБЫ НРМАЛЬН
struct Row {
    int byteValue;
    unsigned long long count;
    double probability;
};

struct GroupStats {
    unsigned long long symbols;
    unsigned long long others;
};

 bool is_symbol(int x) {
    if (x >= '0' && x <= '9') return true;
    if (x >= 'A' && x <= 'Z') return true;
    if (x >= 'a' && x <= 'z') return true;
    return false;
}

void byte_to_hex(int x, char* out) {
    const char* hex = "0123456789ABCDEF";
    out[0] = hex[(x >> 4) & 15];
    out[1] = hex[x & 15];
    out[2] = '\0';
}

 bool read_counts(const std::string& fileName,
                        unsigned long long counts[256],
                        unsigned long long& total) {
    for (int i = 0; i < 256; i++) counts[i] = 0;
    total = 0;

    std::ifstream f(fileName.c_str(), std::ios::binary);
    if (!f) return false;

    char buf[4096];
    while (true) {
        f.read(buf, sizeof(buf));
        std::streamsize n = f.gcount();
        if (n <= 0) break;
        for (int i = 0; i < n; i++) {
            unsigned char b = (unsigned char)buf[i];
            counts[b]++;
            total++;
        }
    }
    return true;
}

 GroupStats calc_groups(unsigned long long counts[256]) {
    GroupStats g;
    g.symbols = 0;
    g.others = 0;
    for (int i = 0; i < 256; i++) {
        if (is_symbol(i)) g.symbols += counts[i];
        else g.others += counts[i];
    }
    return g;
}

 std::vector<Row> build_rows(unsigned long long counts[256], unsigned long long total) {
    std::vector<Row> rows;
    for (int i = 0; i < 256; i++) {
        if (counts[i] == 0) continue;
        Row r;
        r.byteValue = i;
        r.count = counts[i];
        if (total == 0) r.probability = 0.0;
        else r.probability = (double)counts[i] / (double)total;
        rows.push_back(r);
    }
    return rows;
}

 void draw_pie(int top, int left, int radius, GroupStats g, bool useColor) {
    const double PI = 3.14;
    unsigned long long total = g.symbols + g.others;
    double part = 0.0;
    if (total != 0) part = (double)g.symbols / (double)total;
    double borderAngle = 2.0 * PI * part;

    int cy = top + radius;
    int cx = left + radius * 2;

    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius * 2; x <= radius * 2; x++) {
            double xx = (double)x / 2.0;
            double d2 = xx * xx + (double)(y * y);
            if (d2 > (double)(radius * radius)) continue;

            bool isS = false;
            if (total != 0) {
                double ang = atan2((double)y, xx) + PI / 2.0;
                if (ang < 0.0) ang += 2.0 * PI;
                if (ang < borderAngle) isS = true;
            }

            char ch = isS ? 'S' : 'O';
            if (useColor) attron(COLOR_PAIR(isS ? 2 : 3));
            mvaddch(cy + y, cx + x, ch);
            if (useColor) attroff(COLOR_PAIR(isS ? 2 : 3));
        }
    }
}

 void print_usage(const char* prog) {
    std::cout << "Использование:\n";
    std::cout << "  " << prog << " <имя_файла> -tr <порог>\n";
    std::cout << "Пример:\n";
    std::cout << "  " << prog << " data.bin -tr 0.4\n";
}

 int table_cols_fit(int w) {
    int cell = 8;
    int usable = w - 6;
    if (usable < cell) return 0;
    return usable / cell;
}

 void render_table(const std::vector<Row>& rows,
                         unsigned long long totalBytes,
                         double threshold,
                         bool mode) {
    int h, w;
    getmaxyx(stdscr, h, w);
    clear();

    mvprintw(0, 0, "Просмотр частот байтов");
    mvprintw(1, 0, "Всего байт: %llu", totalBytes);
    if (mode == 0) mvprintw(2, 0, "Режим: вероятность | Порог: %.3f", threshold);
    else mvprintw(2, 0, "Режим: количество   | Порог: %.3f", threshold);
    mvprintw(3, 0, "Клавиши: c=кол-во g=диаграмма q=выход");
    mvhline(4, 0, '-', w > 0 ? w : 1);

    int cell = 8;
    int cols = table_cols_fit(w);
    int top = 7;
    int left = 2;

    if (cols <= 0 || h < 13) {
        mvprintw(7, 0, "Окно слишком маленькое для таблицы");
        refresh();
        return;
    }

    mvprintw(5, 0, "Таблица:");

    int rowsPerBlock = 5;
    int blockSpace = 1;
    int blocksFit = (h - top - 2 + blockSpace) / (rowsPerBlock + blockSpace);
    if (blocksFit < 1) blocksFit = 1;

    int allShown = 0;
    for (int b = 0; b < blocksFit; b++) {
        int y = top + b * (rowsPerBlock + blockSpace);
        if (y + 4 >= h - 1) break;
        int start = b * cols;
        if (start >= (int)rows.size()) break;

        int shown = cols;
        if (shown > (int)rows.size() - start) shown = (int)rows.size() - start;

        mvhline(y, left, '-', cell * cols + 1);
        for (int i = 0; i <= cols; i++) {
            mvaddch(y, left + i * cell, '+');
            mvaddch(y + 2, left + i * cell, '+');
            mvaddch(y + 4, left + i * cell, '+');
        }
        mvhline(y + 2, left, '-', cell * cols + 1);
        mvhline(y + 4, left, '-', cell * cols + 1);
        for (int i = 0; i < cols; i++) {
            mvaddch(y + 1, left + i * cell, '|');
            mvaddch(y + 3, left + i * cell, '|');
        }
        mvaddch(y + 1, left + cols * cell, '|');
        mvaddch(y + 3, left + cols * cell, '|');

        for (int i = 0; i < shown; i++) {
            int idx = start + i;
            const Row& r = rows[idx];
            int x = left + i * cell + 2;

            char hx[3];
            byte_to_hex(r.byteValue, hx);
            mvprintw(y + 1, x, "%s", hx);

            bool hi = r.probability >= threshold;
            if (hi && has_colors()) attron(COLOR_PAIR(1));
            if (mode == 0) mvprintw(y + 3, x - 1, "%.2f", r.probability);
            else mvprintw(y + 3, x - 1, "%llu", r.count);
            if (hi && has_colors()) attroff(COLOR_PAIR(1));
        }

        allShown += shown;
    }

    mvhline(h - 1, 0, '-', w > 0 ? w : 1);
    if (rows.size() == 0) mvprintw(h - 1, 2, "Показано: 0/0");
    else mvprintw(h - 1, 2, "Показано: %d/%d", allShown, (int)rows.size());
    refresh();
}

 void render_chart(GroupStats groups) {
    int h, w;
    getmaxyx(stdscr, h, w);
    clear();

    mvprintw(0, 0, "Круговая диаграмма");
    mvprintw(1, 0, "Нажмите g, чтобы вернуться к таблице");
    mvhline(2, 0, '-', w > 0 ? w : 1);

    int radius = (h - 10) / 2;
    if (radius > (w - 10) / 4) radius = (w - 10) / 4;
    if (radius > 12) radius = 12;

    int top = 4;
    bool useColor = has_colors();

    unsigned long long all = groups.symbols + groups.others;
    double p1 = 0.0;
    if (all != 0) p1 = 100.0 * (double)groups.symbols / (double)all;
    double p2 = 100.0 - p1;

    if (radius >= 2) {
        int left = (w - radius * 4) / 2;
        if (left < 0) left = 0;
        draw_pie(top, left, radius, groups, useColor);
    } else {
        int barW = w - 16;
        if (barW < 5) barW = 5;
        int sFill = (int)(p1 * barW / 100.0);
        if (sFill < 0) sFill = 0;
        if (sFill > barW) sFill = barW;
        int oFill = barW - sFill;


        mvprintw(6, 0, "S ");
        for (int i = 0; i < sFill; i++) mvaddch(6, 2 + i, '#');
        mvprintw(6, 3 + barW, "%.1f%%", p1);

        mvprintw(7, 0, "O ");
        for (int i = 0; i < oFill; i++) mvaddch(7, 2 + i, '#');
        mvprintw(7, 3 + barW, "%.1f%%", p2);
    }

    int y = top + radius * 2 + 2;
    if (radius < 2) y = 10;
    if (y < h - 2) {
        if (useColor) attron(COLOR_PAIR(2));
        mvprintw(y, 0, "S");
        if (useColor) attroff(COLOR_PAIR(2));
        mvprintw(y, 2, "Буквы/цифры: %.2f%%", p1);
    }
    if (y + 1 < h - 1) {
        if (useColor) attron(COLOR_PAIR(3));
        mvprintw(y + 1, 0, "O");
        if (useColor) attroff(COLOR_PAIR(3));
        mvprintw(y + 1, 2, "Остальные: %.2f%%", p2);
    }

    refresh();
}

int main(int argc, char* argv[]) {
    if (argc != 4 || strcmp(argv[2], "-tr") != 0) {
        print_usage(argv[0]);
        return 1;
    }

    char* endPtr = nullptr;
    double threshold = strtod(argv[3], &endPtr);
    if (endPtr == argv[3] || *endPtr != '\0') {
        std::cerr << "Ошибка: порог должен быть числом\n";
        return 1;
    }
    if (threshold < 0.0 || threshold > 1.0) {
        std::cerr << "Ошибка: порог должен быть в диапазоне [0, 1]\n";
        return 1;
    }

    unsigned long long counts[256];
    unsigned long long totalBytes = 0;
    if (!read_counts(argv[1], counts, totalBytes)) {
        std::cerr << "Ошибка: не удалось открыть файл: " << argv[1] << "\n";
        return 1;
    }

    std::vector<Row> rows = build_rows(counts, totalBytes);
    GroupStats groups = calc_groups(counts);

    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_GREEN, -1);
        init_pair(2, COLOR_CYAN, -1);
        init_pair(3, COLOR_YELLOW, -1);
    }

    int mode = false; 
    bool showChart = false;

    while (true) {
        if (showChart) render_chart(groups);
        else render_table(rows, totalBytes, threshold, mode);

        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        if (ch == 'g' || ch == 'G') {
            showChart = !showChart;
            continue;
        }

        if ((!showChart) && (ch == 'c' || ch == 'C')){
            mode = !mode;
        }
    }

    endwin();
    return 0;
}