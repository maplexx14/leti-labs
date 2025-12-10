#include <ncurses.h>
#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int stringToInt(char *str, int len) {
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
double stringToDouble(char *str, int len){
    double result = 0;
    int index = 1;
    bool sot = false;
    bool minus = false;
    for (int i =0; i  < len; i++){
        if (str[i] == '.'){
            sot = true;
            continue;
        }
        if (str[i] == '-'){
            minus = true;
        }

        if (sot && str[i] >= '0' && str[i] <= '9'){
            result = result + (str[i]-'0') / pow(10,index);
            index++;
            continue;
        }
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0'); 
            continue;
        }
    }
    if (minus) return result * -1;
    return result;
}

char* usInput() {
    char* input = malloc(31);//выделяем память для 31 элемента и не инициализируем
    int index = 0;
    char choice;
    while (index < 30 && (choice = getch()) != '\n') {
        input[index++] = choice;
    }
    input[index] = '\0';//первый элемент
    return input;
}
int main(){
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    //noecho();
    int choice;
    while (choice != '0'){
        refresh();
        clear();
        printw("Выберите действие:\n1.Ввести число\n2.Ввести число с плавающей точкой\n3.Строку\n\r0.Выход\n\r");
        choice = getch();
        refresh();
        printw("Выбрано %c\n", choice);
        refresh();
        char* inputStr;
        long long num;
        double numf;
        int length = 30;
        switch (choice)
        {
            case '1':
                printw("Введите число:\n");
                refresh();
                inputStr = usInput();
                num = stringToInt(inputStr, length);
                printw("Введено число: %d\n", num);
                printw("Число с учетом ограничений: %d\n", num * -2);
                free(inputStr);
                getch();
                break;
            case '2':
                printw("Введите число с плавающей точкой:\n");
                refresh();
                inputStr = usInput();
                numf = stringToDouble(inputStr, length);
                printw("Введено число с плавающей точкой: %f\n", numf);
                printw("Число с учетом ограничений: %f\n", numf * -2);
                free(inputStr);
                getch();
                break;
            case '3':
                printw("Введите строку:\n");
                refresh();
                inputStr = usInput();
                printw("Введена строка(с учетом ограничений): ");
                for(int i =0; i <= 30;i++){
                    if(!((inputStr[i] >= 'A' && inputStr[i] <= 'Z'))){
                        printw("%c", inputStr[i]);
                    }
                }
                free(inputStr);
                getch();
                break;
            
        }
        refresh();
    }
    return 0;
}
