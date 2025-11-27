#include "library.h"
#include <stdlib.h>

// макрос для кроссплатформенности
// В Windows нужен __declspec(dllexport) для экспорта функций из DLL
// В Linux/Unix функции экспортируются по умолчанию
// макрос EXPORT - видим функции вне библиотеки
#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif

// Реализация №1: e = (1 + 1/x)^x
EXPORT float e(int x) {
    if (x <= 0) return 0.0f;
    float temp = 1.0f + 1.0f / (float)x;
    float result = 1.0f;
    
    // Возведение в степень через умножение
    for (int i = 0; i < x; i++) {
        result *= temp;
    }
    return result;
}

// Реализация №1: Перевод в двоичную систему
EXPORT char* convert(int x) {
    if (x == 0) {
        char* result = malloc(2);
        result[0] = '0';
        result[1] = '\0';
        return result;
    }
    
    // Определяем длину результата
    int temp = abs(x);
    int length = 0;
    while (temp > 0) {
        temp /= 2;
        length++;
    }
    
    // Добавляем место для знака и нуль-терминатора
    if (x < 0) length++;
    char* result = malloc(length + 1);
    result[length] = '\0';
    
    // Заполняем строку с конца
    temp = abs(x);
    int index = length - 1;
    while (temp > 0) {
        result[index--] = (temp % 2) + '0';
        temp /= 2;
    }
    
    // Добавляем знак если нужно
    if (x < 0) {
        result[0] = '-';
    }
    
    return result;
}