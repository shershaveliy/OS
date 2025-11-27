#include <stdlib.h>
#include <string.h>
#include <unistd.h>// для read(), write(), STDIN_FILENO, STDOUT_FILENO

// Функции из библиотеки (линкуются на этапе компиляции)
// Компоновщик найдет эти функции в library1.so
extern float e(int x);
extern char* convert(int x);

void print(const char* msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

void print_error(const char* msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

char* int_to_string(int value) {
    if (value == 0) {
        char* result = malloc(2);
        result[0] = '0';
        result[1] = '\0';
        return result;
    }
    
    int is_negative = value < 0;
    unsigned int num = is_negative ? -value : value;
    int length = 0;
    unsigned int temp = num;
    
    while (temp > 0) {
        temp /= 10;
        length++;
    }
    
    if (is_negative) length++;
    char* result = malloc(length + 1);
    result[length] = '\0';
    
    temp = num;
    int index = length - 1;
    while (temp > 0) {
        result[index--] = (temp % 10) + '0';
        temp /= 10;
    }
    
    if (is_negative) {
        result[0] = '-';
    }
    
    return result;
}

// Конвертация float в строку (упрощенная)
char* float_to_string(float value) {
    // Целая часть
    int int_part = (int)value;
    // Дробная часть (2 знака)
    int frac_part = (int)((value - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    
    char* int_str = int_to_string(int_part);
    char* result = malloc(strlen(int_str) + 5); // целая часть + точка + 2 цифры + '\0'
    
    strcpy(result, int_str);
    strcat(result, ".");
    
    // Добавляем дробную часть
    if (frac_part < 10) {
        strcat(result, "0");
    }
    char frac_str[3];
    frac_str[0] = (frac_part / 10) + '0';
    frac_str[1] = (frac_part % 10) + '0';
    frac_str[2] = '\0';
    strcat(result, frac_str);
    
    free(int_str);
    return result;
}

//Замена atoi
int parse_int(const char* str, int* result) {
    if (str == NULL || *str == '\0') return 0;
    
    int value = 0;
    int sign = 1;
    int i = 0;
    
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }
    
    // Парсинг цифр
    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            value = value * 10 + (str[i] - '0');
        } else {
            return 0; // Не цифра
        }
    }
    
    *result = value * sign;
    return 1;
}

int main() {
    print("Программа №1 - Статическая линковка\n");
    print("Доступные команды:\n");
    print("0 - информация о реализации\n");
    print("1 x - вычисление числа e для x\n");
    print("2 x - конвертация числа x\n");
    print("exit - выход из программы\n\n");
    
    char buffer[256];
    char* token;
    
    while (1) {
        print("> ");
        
        // Чтение ввода
        ssize_t bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) break;
        
        buffer[bytes] = '\0';
        
        // Удаление символа новой строки
        if (bytes > 0 && buffer[bytes - 1] == '\n'){
            buffer[bytes - 1] = '\0';
        }
        
        // Парсинг команды
        token = strtok(buffer, " ");
        if (token == NULL) continue;
        
        if (strcmp(token, "exit") == 0){
            break;
        }
        else if (strcmp(token, "0") == 0){
            print("Используется реализация №1 (статическая линковка)\n");
            print("e(x) = (1 + 1/x)^x\n");
            print("convert(x) = перевод в двоичную систему\n");
        }
        else if (strcmp(token, "1") == 0){
            token = strtok(NULL, " ");
            if (token == NULL) {
                print_error("error: missing argument for command '1'\n");
                continue;
            }
            
            int x;
            if (!parse_int(token, &x)) {
                print_error("error: invalid integer argument\n");
                continue;
            }
            
            float result = e(x);
            char* result_str = float_to_string(result);
            
            char* x_str = int_to_string(x);
            char output[256];
            strcpy(output, "e(");
            strcat(output, x_str);
            strcat(output, ") = ");
            strcat(output, result_str);
            strcat(output, "\n");
            
            print(output);
            
            free(x_str);
            free(result_str);
        }
        else if (strcmp(token, "2") == 0){
            token = strtok(NULL, " ");
            if (token == NULL) {
                print_error("error: missing argument for command '2'\n");
                continue;
            }
            
            int x;
            if (!parse_int(token, &x)){
                print_error("error: invalid integer argument\n");
                continue;
            }
            
            char* result = convert(x);
            
            char* x_str = int_to_string(x);
            char output[256];
            strcpy(output, "convert(");
            strcat(output, x_str);
            strcat(output, ") = ");
            strcat(output, result);
            strcat(output, "\n");
            
            print(output);
            
            free(x_str);
            free(result);
        }
        else {
            print_error("error: unknown command\n");
        }
    }
    
    return 0;
}