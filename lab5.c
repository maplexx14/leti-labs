#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <unistd.h>

void genField(int matrix[][20], int sizeX, int sizeY) {
    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            matrix[i][j] = rand() % 2;
        }
    }
}

int countNeighbors(int matrix[][20], int x, int y, int sizeX, int sizeY) {
    int count = 0;
    

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            
            int nx = x + i;
            int ny = y + j;
            
            if (nx >= 0 && nx < sizeX && ny >= 0 && ny < sizeY) {
                count += matrix[nx][ny];
            }
        }
    }
    return count;
}

void nextGeneration(int matrix[][20], int sizeX, int sizeY) {
    int newMatrix[20][20];
    

    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            int neighbors = countNeighbors(matrix, i, j, sizeX, sizeY);
    
            if (matrix[i][j] == 1) {
        
                if (neighbors < 2 || neighbors > 3) {
                    newMatrix[i][j] = 0;
                } else {
                    newMatrix[i][j] = 1;
                }
            } else {
              
                if (neighbors == 3) {
                    newMatrix[i][j] = 1;
                } else {
                    newMatrix[i][j] = 0; 
                }
            }
        }
    }

    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            matrix[i][j] = newMatrix[i][j];
        }
    }
}

void displayMatrix(int matrix[][20], int sizeX, int sizeY, int c) {
    clear();
    printw("Поколение: %d\n", c);
    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            matrix[i][j] ? printw("█") : printw(" ");
        }


        printw("\n");
    }
    refresh();
}

int main() {
    int matrix[20][20];
    int sizeX = 20;
    int sizeY = 20;
    
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(0);
    srand(time(NULL));
    
    genField(matrix, sizeX, sizeY);
    int c = 0;
    while (1) {
        
        displayMatrix(matrix, sizeX, sizeY, c);
        nextGeneration(matrix, sizeX, sizeY);
        usleep(100000);
        timeout(0);
        c++;
        if (getch() == 'q') {
            break;
        }
    }
    
    endwin();
    return 0;
}