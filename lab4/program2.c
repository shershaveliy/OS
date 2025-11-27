#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h> // Для динамической подгрузки библиотек

typedef float e_func(int x);
typedef char* convert_func(int x);

// Структура для библиотеки
typedef struct {
    void* handle;// Указатель на загруженную библиотеку
    e_func* e_ptr;// Указатель на функцию e
    convert_func* convert_ptr;// Указатель на функцию convert
    const char* description;// Описание реализации
} Library;

void print(const char* msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

void print_error(const char* msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

// Заглушки для функций - вызывается если библиотека не загружена
static float e_stub(int x) {
    print_error("error: function e not loaded\n");
    return 0.0f;
}

static char* convert_stub(int x) {
    print_error("error: function convert not loaded\n");
    char* result = malloc(2);
    result[0] = '0';
    result[1] = '\0';
    return result;
}

// Конвертация чисел в строки (аналогично program1.c)
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

char* float_to_string(float value) {
    int int_part = (int)value;
    int frac_part = (int)((value - int_part) * 100);
    if (frac_part < 0) frac_part = -frac_part;
    
    char* int_str = int_to_string(int_part);
    char* result = malloc(strlen(int_str) + 5);
    
    strcpy(result, int_str);
    strcat(result, ".");
    
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
    
    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            value = value * 10 + (str[i] - '0');
        } else {
            return 0;
        }
    }
    
    *result = value * sign;
    return 1;
}

int load_library(Library* lib, const char* path){
    //Загрузка библиотеки в память
    lib->handle = dlopen(path, RTLD_LAZY);// RTLD_LAZY - ленивая загрузка символов
    if (!lib->handle) {
        return 0;
    }
    
    lib->e_ptr = (e_func*)dlsym(lib->handle, "e");// Получение указателей на функции
    lib->convert_ptr = (convert_func*)dlsym(lib->handle, "convert");
    // Проверяем что обе функции найдены
    if (!lib->e_ptr || !lib->convert_ptr) {
        dlclose(lib->handle);
        return 0;
    }
    
    return 1;
}

void print_library_info(int index, const Library* lib){
    char num_str[2];
    num_str[0] = '1' + index;
    num_str[1] = '\0';
    
    print("Текущая реализация: ");
    print(num_str);
    print("\n");
    print(lib->description);
    print("\n\n");
}

int main() {
    Library libs[2]; //Массив для двух библиотек
    int current = 0; //Индекс текущей библиотеки 
    
    // Инициализация библиотек
    libs[0].handle = NULL;
    libs[0].e_ptr = e_stub;
    libs[0].convert_ptr = convert_stub;
    libs[0].description = "e(x) = (1 + 1/x)^x\nconvert(x) = binary";
    
    libs[1].handle = NULL;
    libs[1].e_ptr = e_stub;
    libs[1].convert_ptr = convert_stub;
    libs[1].description = "e(x) = sum(1/n!) from n=0 to x\nconvert(x) = ternary";
    
    // Загрузка первой библиотеки по умолчанию
    if (!load_library(&libs[0], "./library1.so")) {
        print_error("error: failed to load library1.so\n");
    }
    
    print("Программа №2 - Динамическая загрузка\n");
    print("Доступные команды:\n");
    print("0 - переключение реализации\n");
    print("1 x - вычисление числа e для x\n");
    print("2 x - конвертация числа x\n");
    print("exit - выход из программы\n\n");
    
    print_library_info(current, &libs[current]);
    
    char buffer[256];
    char* token;
    
    while (1) {
        print("> ");
        
        ssize_t bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) break;
        
        buffer[bytes] = '\0';
        
        if (bytes > 0 && buffer[bytes - 1] == '\n') {
            buffer[bytes - 1] = '\0';
        }
        
        token = strtok(buffer, " ");
        if (token == NULL) continue;
        
        if (strcmp(token, "exit") == 0) {
            break;
        }
        else if (strcmp(token, "0") == 0) {
            current = (current + 1) % 2;
            
            // Пытаемся загрузить библиотеку если она еще не загружена
            if (!libs[current].handle) {
                const char* path = (current == 0) ? "./library1.so" : "./library2.so";
                if (!load_library(&libs[current], path)) {
                    print_error("error: failed to load library\n");
                    libs[current].e_ptr = e_stub;
                    libs[current].convert_ptr = convert_stub;
                }
            }
            
            print_library_info(current, &libs[current]);
        }
        else if (strcmp(token, "1") == 0) {
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
            //Вызов функции через указатель
            float result = libs[current].e_ptr(x);

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
        else if (strcmp(token, "2") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                print_error("error: missing argument for command '2'\n");
                continue;
            }
            
            int x;
            if (!parse_int(token, &x)) {
                print_error("error: invalid integer argument\n");
                continue;
            }
            
            char* result = libs[current].convert_ptr(x);
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
    
    // Закрытие библиотек
    for (int i = 0; i < 2; i++) {
        if (libs[i].handle) {
            dlclose(libs[i].handle);
        }
    }
    
    return 0;
}