#include "library.h"
#include <stdlib.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

// Реализация №2: e = сумма ряда 1/n! от n=0 до x
EXPORT float e(int x) {
    if (x < 0) return 0.0f;
    
    float result = 0.0f;
    float factorial = 1.0f;
    
    for (int n = 0; n <= x; n++) {
        if (n > 0) {
            factorial *= n;
        }
        result += 1.0f / factorial;
    }
    return result;
}

// Реализация №2: Перевод в троичную систему
EXPORT char* convert(int x) {
    if (x == 0) {
        char* result = malloc(2);
        result[0] = '0';
        result[1] = '\0';
        return result;
    }
    
    int temp = abs(x);
    int length = 0;
    while (temp > 0) {
        temp /= 3;
        length++;
    }
    
    if (x < 0) length++;
    char* result = malloc(length + 1);
    result[length] = '\0';
    
    temp = abs(x);
    int index = length - 1;
    while (temp > 0) {
        result[index--] = (temp % 3) + '0';
        temp /= 3;
    }
    
    if (x < 0) {
        result[0] = '-';
    }
    
    return result;
}