#include <ncurses.h>
#include <locale.h>
#include <math.h>
#include <fstream>

#define MAX_BOOKS 100
#define MAX_STR_LEN 100

struct bookInfo {
    int id;
    char author[MAX_STR_LEN];
    char name[MAX_STR_LEN];
    double price;
    char dateIncome[11];
    int loyaltySys;
    int quantity;
};

int booksCount = 0;
int nextId = 1;
struct bookInfo records[MAX_BOOKS];

int myStrlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void printStruct(struct bookInfo b) {
    printw("ID: %d\n", b.id);
    printw("Автор: %s\n", b.author);
    printw("Название: %s\n", b.name);
    printw("Цена: %.2lf\n", b.price);
    printw("Дата поступления: %s\n", b.dateIncome);
    printw("Система лояльности: %s\n", b.loyaltySys ? "Да" : "Нет");
    printw("Количество: %d\n", b.quantity);
    printw("-------------------------\n");
}

double stringToDouble(char *str) {
    double result = 0;
    int index = 0;
    int decimal = 0;
    int decimalCount = 0;
    int sign = 1;
    
    if (str[0] == '-') {
        sign = -1;
        index = 1;
    }
    
    while (str[index] != '\0') {
        if (str[index] == '.') {
            if (decimal) return 0;
            decimal = 1;
            index++;
            continue;
        }
        
        if (str[index] >= '0' && str[index] <= '9') {
            if (!decimal) {
                result = result * 10 + (str[index] - '0');
            } else {
                decimalCount++;
                result = result + (str[index] - '0') / pow(10, decimalCount);
            }
        } else {
            return 0;
        }
        index++;
    }
    
    return result * sign;
}

int stringToInt(char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }
    
    while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        } else {
            return 0;
        }
        i++;
    }
    
    return result * sign;
}

int isValidDate(char *date) {
    if (myStrlen(date) != 10) return 0;
    if (date[2] != '.' || date[5] != '.') return 0;
    
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (date[i] < '0' || date[i] > '9') return 0;
    }
    
    char dayStr[3] = {date[0], date[1], '\0'};
    char monthStr[3] = {date[3], date[4], '\0'};
    char yearStr[5] = {date[6], date[7], date[8], date[9], '\0'};
    
    int day = stringToInt(dayStr);
    int month = stringToInt(monthStr);
    int year = stringToInt(yearStr);
    
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
        return 0;
    }
    
    if (month == 2) {
        int isLeap = 0;
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            isLeap = 1;
        }
        
        if (isLeap && day > 29) return 0;
        if (!isLeap && day > 28) return 0;
    }
    
    if (year < 1900 || year > 2100) return 0;
    
    return 1;
}

void inputString(char *buffer, int maxLen, const char *prompt) {
    printw("%s: ", prompt);
    refresh();
    
    echo();
    int pos = 0;
    int ch;
    
    while (pos < maxLen - 1) {
        ch = getch();
        
        if (ch == 127 || ch == '\b') {
            if (pos > 0) {
                pos--;
                addstr("\b \b");
                refresh();
            }
        } else if (ch >= 32 && ch <= 126) {
            buffer[pos++] = ch;
        }
    }
    
    buffer[pos] = '\0';
    noecho();
    printw("\n");
}

void inputInt(char *buffer, int maxLen, const char *prompt) {
    printw("%s: ", prompt);
    refresh();
    
    echo();
    int pos = 0;
    int ch;
    
    while (pos < maxLen - 1) {
        ch = getch();
        
        if (ch == '\n' || ch == '\r') {
            break;
        }
        
        if (ch == 127 || ch == '\b') {
            if (pos > 0) {
                pos--;
                addstr("\b \b");
                refresh();
            }
        } else if (ch >= '0' && ch <= '9') {
            buffer[pos++] = ch;
        } else if (ch == '-' && pos == 0) {
            buffer[pos++] = ch;
        }
    }
    
    buffer[pos] = '\0';
    noecho();
    printw("\n");
}

void inputDouble(char *buffer, int maxLen, const char *prompt) {
    printw("%s: ", prompt);
    refresh();
    
    echo();
    int pos = 0;
    int ch;
    int hasDot = 0;

    while (pos < maxLen - 1) {
        ch = getch();
        
        if (ch == '\n' || ch == '\r') {
            break;
        }
        
        if (ch == 127 || ch == '\b') {
            if (pos > 0) {
                pos--;
                if (buffer[pos] == '.') {
                    hasDot = 0;
                }
                addstr("\b \b");
                refresh();
            }
        } else if (ch >= '0' && ch <= '9') {
            buffer[pos++] = ch;
        } else if (ch == '.' && !hasDot) {
            buffer[pos++] = ch;
            hasDot = 1;
        } else if (ch == '-' && pos == 0) {
            buffer[pos++] = ch;
        }
    }
    
    buffer[pos] = '\0';
    noecho();
    printw("\n");
}

int isValidPrice(char *priceStr) {
    if (myStrlen(priceStr) == 0) return 0;
    
    int hasDot = 0;
    int i = 0;
    
    if (priceStr[0] == '-') i = 1;
    
    for (; priceStr[i] != '\0'; i++) {
        if (priceStr[i] == '.') {
            if (hasDot) return 0;
            hasDot = 1;
        } else if (priceStr[i] < '0' || priceStr[i] > '9') {
            return 0;
        }
    }
    
    double price = stringToDouble(priceStr);
    if (price <= 0) return 0;
    
    return 1;
}

void addToSys() {
    clear();
    
    if (booksCount >= MAX_BOOKS) {
        printw("Достигнут лимит книг\n");
        printw("Нажмите любую клавишу для продолжения");
        refresh();
        getch();
        return;
    }
    
    struct bookInfo newBook;
    newBook.id = nextId++;
    
    inputString(newBook.author, MAX_STR_LEN, "Введите автора");
    inputString(newBook.name, MAX_STR_LEN, "Введите название");
    
    char priceStr[20];
    int validPrice = 0;
    while (!validPrice) {
        inputDouble(priceStr, 20, "Введите цену (только число)");
        validPrice = isValidPrice(priceStr);
        if (!validPrice) {
            printw("Неверный формат цены. Введите положительное число\n");
            refresh();
        }
    }
    newBook.price = stringToDouble(priceStr);
    
    int validDate = 0;
    while (!validDate) {
        inputString(newBook.dateIncome, 11, "Введите дату поступления (дд.мм.гггг)");
        validDate = isValidDate(newBook.dateIncome);
        if (!validDate) {
            printw("Неверный формат даты. Используйте дд.мм.гггг\n");
            refresh();
        }
    }
    
    char loyaltyStr[2];
    int validLoyalty = 0;
    while (!validLoyalty) {
        printw("Система лояльности (1-Да, 0-Нет): ");
        refresh();
        echo();
        loyaltyStr[0] = getch();
        loyaltyStr[1] = '\0';
        noecho();
        printw("\n");
        
        if (loyaltyStr[0] == '0' || loyaltyStr[0] == '1') {
            validLoyalty = 1;
            newBook.loyaltySys = loyaltyStr[0] - '0';
        } else {
            printw("Введите 0 или 1\n");
            refresh();
        }
    }
    
    char quantityStr[10];
    int validQuantity = 0;
    while (!validQuantity) {
        inputInt(quantityStr, 10, "Введите количество");
        int quantity = stringToInt(quantityStr);
        if (quantity > 0) {
            validQuantity = 1;
            newBook.quantity = quantity;
        } else {
            printw("Количество должно быть положительным числом\n");
            refresh();
        }
    }
    
    records[booksCount] = newBook;
    booksCount++;
    
    printw("\nКнига успешно добавлена. ID: %d\n", newBook.id);
    printw("Нажмите любую клавишу для продолжения");
    refresh();
    getch();
}

void deleteFromSys() {
    clear();
    
    if (booksCount == 0) {
        printw("Система пуста\n");
        printw("Нажмите любую клавишу для продолжения");
        refresh();
        getch();
        return;
    }
    
    printw("Текущие книги:\n");
    for (int i = 0; i < booksCount; i++) {
        printw("%d. %s - %s (ID: %d)\n", i + 1, records[i].author, records[i].name, records[i].id);
    }
    
    printw("\nВведите номер книги для удаления (1-%d): ", booksCount);
    refresh();
    
    char indexStr[10];
    inputInt(indexStr, 10, "");
    int index = stringToInt(indexStr) - 1;
    
    if (index < 0 || index >= booksCount) {
        printw("Неверный номер\n");
    } else {
        printw("Удаляю книгу: %s - %s\n", records[index].author, records[index].name);
        for (int i = index; i < booksCount - 1; i++) {
            records[i] = records[i + 1];
        }
        booksCount--;
        printw("Книга удалена\n");
    }
    
    printw("Нажмите любую клавишу для продолжения");
    refresh();
    getch();
}

void printAllInfo() {
    clear();
    
    if (booksCount == 0) {
        printw("Система пуста\n");
    } else {
        for (int i = 0; i < booksCount; i++) {
            printStruct(records[i]);
        }
    }
    
    printw("\nНажмите любую клавишу для продолжения");
    refresh();
    getch();
}

void saveToFile() {
    clear();
    
    std::ofstream file("books.dat", std::ios::binary);
    if (!file) {
        printw("Ошибка открытия файла для записи\n");
        printw("Нажмите любую клавишу для продолжения");
        refresh();
        getch();
        return;
    }
    
    file.write((char*)&booksCount, sizeof(booksCount));
    file.write((char*)&nextId, sizeof(nextId));
    
    if (booksCount > 0) {
        file.write((char*)records, sizeof(bookInfo) * booksCount);
    }
    
    file.close();
    
    printw("Данные сохранены в файл 'books.dat'\n");
    printw("Сохранено книг: %d\n", booksCount);
    printw("Нажмите любую клавишу для продолжения");
    refresh();
    getch();
}

void loadFromFile() {
    clear();
    
    std::ifstream file("books.dat", std::ios::binary);
    if (!file) {
        printw("Файл 'books.dat' не найден\n");
        printw("Нажмите любую клавишу для продолжения");
        refresh();
        getch();
        return;
    }
    
    file.read((char*)&booksCount, sizeof(booksCount));
    file.read((char*)&nextId, sizeof(nextId));
    
    if (booksCount > 0 && booksCount <= MAX_BOOKS) {
        file.read((char*)records, sizeof(bookInfo) * booksCount);
    }
    
    file.close();
    
    printw("Данные загружены из файла 'books.dat'\n");
    printw("Загружено книг: %d\n", booksCount);
    printw("Следующий ID: %d\n", nextId);
    printw("Нажмите любую клавишу для продолжения");
    refresh();
    getch();
}

void printMenu() {
    clear();
    printw("=== ИНФОРМАЦИОННАЯ СИСТЕМА БИБЛИОТЕКИ ===\n");
    printw("-----------------------------------------\n");
    printw("| 1. Добавить книгу в систему          |\n");
    printw("| 2. Удалить книгу из системы          |\n");
    printw("| 3. Отобразить все книги              |\n");
    printw("| 4. Сохранить данные в файл           |\n");
    printw("| 5. Загрузить данные из файла         |\n");
    printw("| 6. Выход из программы                |\n");
    printw("-----------------------------------------\n");
    printw("\nВыберите действие (1-6): ");
    refresh();
}

int main() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    int running = 1;
    
    while (running) {
        printMenu();
        
        int choice = getch();
        
        switch (choice) {
            case '1':
                addToSys();
                break;
            case '2':
                deleteFromSys();
                break;
            case '3':
                printAllInfo();
                break;
            case '4':
                saveToFile();
                break;
            case '5':
                loadFromFile();
                break;
            case '6':
                running = 0;
                break;
            default:
                break;
        }
    }
    
    endwin();
    return 0;
}