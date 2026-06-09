#include <math.h>
#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>

double stringToDouble(char *str, int len){
    double result = 0;
    int index = 1;
    bool sot = false;
    bool minus = false;
    for (int i =0; i  < len; i++){
        if (str[i] == '\0') break;
        if (str[i] == '.'){
            sot = true;
            continue;
        }
        if (str[i] == '-'){
            minus = true;
            continue; 
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

double manLog(double x) {
    if (x <= 0) {
        return -1; 
    }
    double y = x - 1;
    double term = y;  
    double result = 0.0;
    int n = 1;

    while (term > 0.00001 || term < -0.00001) {
        if (n % 2 == 1) {
            result += term / n;
        } else {
            result -= term / n;
        }
        term *= y;
        n++;
    }
    return result;
}

double powerOf(double num, int power){
    double ans = 1.0;
    for (int i =0;  i < power; i++){
        ans *= num;
    }
    return ans;
}

double sinM(double num){
    double sum = 0.0;
    double term = num;   
    int n = 1;
    int sign = 1;
    while (term > 0.000001 || term < -0.000001) {
        sum += sign * term;
        term *= num * num / ( (2 * n + 1) * (2 * n));
        sign = -sign;
        n++;
    }
    return sum;
}

double cosM(double num){
    double sum = 1.0;
    double term = 1.0;
    int n = 1;
    int sign = -1;
    while (term > 0.000001 || term < -0.000001) {
        term *= num * num / ((2 * n - 1) * (2 * n));
        sum += sign * term;
        sign = -sign;
        n++;
    }
    return sum;
}

double sqrtM(double num){
    if (num < 0){
        return -1;
    } 
    double x = 1;
    for (int i = 0; i < 20; i++) {
        x = 0.5 * (x + num/ x);
    }
    return x;
}

double expM(double num){
    double sum = 1.0;
    double term = 1.0;
    int n =1;
    while(term  > 0.00001 || term < -0.00001){
        term *= num / n;
        sum += term;
        n++;
    }
    return sum;
}

double MathHSolution(double a, double b){
    double ans;
    ans = log(
        (pow(sin(a),2) + cos(b))
        /*----------*/ / /*-------*/
        (sqrt((1 +   ((exp(1)) 
                              / 
                     (pow(a,3) + 3.4 * (b)))))
    ));
    return ans;
}

double manualSolution(double a, double b){
    double ans;
    ans = manLog(
        (powerOf(sinM(a),2) + cosM(b))
        /*----------*/ / /*-------*/
        (sqrtM((1 +   ((expM(1)) 
                              / 
                     (powerOf(a,3) + 3.4 * (b)))))
    ));
    return ans;
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
int main(){
    setlocale(LC_ALL, "");
    initscr();      
    cbreak();
    while(true){
        printw("Введите число a: ");
        refresh();
        char* inputA;
        inputA = usInput();
        double a = stringToDouble(inputA, 100);
        
        printw("Введите число b: ");
        refresh();
        char* inputB;
        inputB = usInput();
        double b = stringToDouble(inputB, 100);

        // через math.h
        double solH = MathHSolution(a, b);
        printw("math.h: %lf\n\r", solH);
        refresh();
        if (isnan(solH) || isinf(solH)){
            printw("ручками: %lf\n\r", solH);
            refresh();
            continue;
        }
        refresh();
        // ручками
        double solM = manualSolution(a,b);
        printw("ручками: %lf\n\r", solM);
        refresh();
        getch();
    }
}
