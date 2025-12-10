#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#define PRINT(x) do { printw("%3d", (x)); } while(0)

int stringToInt(char* str, int len) {
    long long result = 0;
    bool minus = false;
    for (int i = 0; i < len; i++) {
        if (str[i] == '-'){
            minus = true;
        }
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        }
    }
    if (minus) return result * -1;
    return result;
}

char* usInput() {
    char* input = malloc(31);
    int index = 0;
    char choice;
    while (index < 30 && (choice = getch()) != '\n' && choice != '\r') {
        if (input[index-1] == '.'  && choice == '.'){
            addch('\b');
            addch(' ');
            addch('\b');
            refresh();
        }
        if (choice >= '0' && choice <= '9' || choice == '.' || choice == '-') {
            input[index++] = (char)choice;
            refresh();
        }
        
        else {
            addch('\b');
            addch(' ');
            addch('\b');
            refresh();
        }
        
    }
 
    return input;
}

void s_centra(int a[][20], int N) { 
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            a[i][j] = 0;

    int x = (N - 1) / 2;
    int y = (N - 1) / 2;
    int value = 1;

    a[x][y] = value++;
    int step = 1;

    while (value <= N * N) {
        for (int k = 0; k < step && value <= N * N; ++k) {
            y++;
            if (x >= 0 && x < N && y >= 0 && y < N)
                a[x][y] = value++;
        }

        for (int k = 0; k < step && value <= N * N; ++k) {
            x--;
            if (x >= 0 && x < N && y >= 0 && y < N)
                a[x][y] = value++;
        }

        step++;

        for (int k = 0; k < step && value <= N * N; ++k) {
            y--;
            if (x >= 0 && x < N && y >= 0 && y < N)
                a[x][y] = value++;
        }

        for (int k = 0; k < step && value <= N * N; ++k) {
            x++;
            if (x >= 0 && x < N && y >= 0 && y < N)
                a[x][y] = value++;
        }

        step++;
    }
}

void krugom_transponirovan(int m[][20], int tm[][20], int N) {
    for(int i = 0; i < N; ++i){
        for(int j = 0; j < N; ++j)
            tm[i][j] = m[j][i];
    }
}

void gengovno(int rm[][20], int N){      
    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j)
            rm[i][j] = rand() % 10;
}

void vichest_kal(int* tm[][20], int* rm[][20],int N){
    for(int i = 0; i < N; ++i){
        for(int j = 0; j < N; ++j)
            *tm[i][j] -= *rm[i][j];
    }
}

int main() {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    refresh();

    srand(time(NULL));
    printw("Введите число N\n");
    char* s = usInput();
    int N = stringToInt(s, 31);
    free(s);

    int matrix[20][20];
    int tmatrix[20][20];
    int rmatrix[20][20];

    int *tm[20][20];
    int *m[20][20];
    int *rm[20][20];

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            m[i][j]  = &matrix[i][j]; 
            tm[i][j] = &tmatrix[i][j];
            rm[i][j] = &rmatrix[i][j];
        }

 

    s_centra(matrix, N);

    printw("\nОбычная по паттерну 4\n");
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            PRINT(matrix[i][j]);
        printw("\n");
    }

    printw("\n");

    krugom_transponirovan(matrix, tmatrix, N);

    printw("Транспонированная\n");
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            PRINT(tmatrix[i][j]);
        printw("\n");
    }

    gengovno(rmatrix, N);          

    printw("Сгенерированная\n");
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            PRINT(rmatrix[i][j]); 
        printw("\n");
    }

    printw("\nВычтенная\n");
    vichest_kal(tm, rm, N);  

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            PRINT(tmatrix[i][j]); 
        printw("\n");
    }

    refresh();
    getch();
    endwin();
    return 0;
}
