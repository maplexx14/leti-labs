#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>

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

void pakaz(int marsik[][20], int N){
    for(int i = 0; i < N; ++i){
        for(int j = 0; j < N; ++j)
            
            printw("%3d",marsik[j][i]);
        printw("\n");
        }
    printw("\n");
    refresh();
}


int main(){
    setlocale(LC_ALL, "");
    initscr();
    keypad(stdscr, TRUE);
    refresh();
    printw("prog\n");
    int matrix[20][20]  ={};

    int *pm[20][20];
    int currentx = 0;
    int currenty =0;

    char* s = usInput();
    int N = stringToInt(s, 31);
    int usIn;
    int num = 1;
    while ((usIn = getch()) != '\n'){
       
    
        switch (usIn)
        {
            case KEY_UP:
                if (currenty > 0) currenty--;
                break;
            case KEY_DOWN:
                if (currenty < N-1) currenty++; 
                break;
            case KEY_LEFT:
                if (currentx > 0) currentx--;
                break;
            case KEY_RIGHT:
                if (currentx < N-1) currentx++; 
                break;   
            }
        if (matrix[currentx][currenty] != 0){
            printw("лошара");
            break;
        }
        matrix[currentx][currenty] = num;
        erase();
        pakaz(matrix, N);
        num++;
    }

    refresh();
    getch();
    endwin();
    return 0;
}
