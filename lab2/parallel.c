#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include "parallel.h"

typedef struct {
    int *array;
    int left;
    int right;
    int depth;
    int max_threads;
} SortArgs;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int active_threads = 0;

//Функция для получения времени
double get_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);//Системный вызов для получения текущего времени
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Последовательное слияние двух отсортированных массивов
void merge(int *array, int left, int mid, int right){

    int n1 = mid - left + 1;// Размер левого подмассива
    int n2 = right - mid;// Правого
    
    int *left_arr = malloc(n1 * sizeof(int));
    int *right_arr = malloc(n2 * sizeof(int));
    
    for (int i = 0; i < n1; i++){
        left_arr[i] = array[left + i];
    }
    for (int j = 0; j < n2; j++){
        right_arr[j] = array[mid + 1 + j];
    }
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2){
        if (left_arr[i] <= right_arr[j]){
            array[k] = left_arr[i];
            i++;
        }
        else{
            array[k] = right_arr[j];
            j++;
        }
        k++;
    }
    //Копируем оставшиеся элементы из левого массива, на случай
    //если правый закончился раньше
    while (i < n1){
        array[k] = left_arr[i];
        i++;
        k++;
    }
    
    while (j < n2){
        array[k] = right_arr[j];
        j++;
        k++;
    }
    
    free(left_arr);
    free(right_arr);
}

// Последовательная сортировка слиянием
void sequential_merge_sort(int *array, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        
        sequential_merge_sort(array, left, mid);
        sequential_merge_sort(array, mid + 1, right);
        
        merge(array, left, mid, right);
    }
}

// Функция, выполняемая в потоке для сортировки
static void *parallel_merge_sort_thread(void *_args){
    //Явное преобразования типа
    SortArgs *args = (SortArgs *)_args;
    int *array = args->array;
    int left = args->left;
    int right = args->right;
    int depth = args->depth;
    int max_threads = args->max_threads;
    
    if (left >= right || (right - left) < 1000){
        // условия выхода из рекурсии:
        // 1. left >= right - массив пустой или из одного элемента
        // 2. (right - left) < 1000 - массив достаточно маленький используем
        // Последовательную сортировку
        sequential_merge_sort(array, left, right);
        free(args);
        return NULL;
    }
    
    int mid = left + (right - left) / 2;
    
    SortArgs *left_args = malloc(sizeof(SortArgs));
    SortArgs *right_args = malloc(sizeof(SortArgs));
    
    left_args->array = array;
    left_args->left = left;
    left_args->right = mid;
    left_args->depth = depth + 1;
    left_args->max_threads = max_threads;
    
    right_args->array = array;
    right_args->left = mid + 1;
    right_args->right = right;
    right_args->depth = depth + 1;
    right_args->max_threads = max_threads;
    
    pthread_t left_thread; // Идентификатор потока для левой половины
    int left_created = 0; // Флаг создания потока (0/1)
    
    pthread_mutex_lock(&mutex);
    if (active_threads < max_threads){
        active_threads++;
        left_created = 1;
    }
    pthread_mutex_unlock(&mutex);
    
    if (left_created){
        pthread_create(&left_thread, NULL, parallel_merge_sort_thread, left_args);
        //NULL - атрибуты потока по умолчанию
    }
    else{
        //Лимит потоков исчерпан, вызываем рекурсию
        parallel_merge_sort_thread(left_args);
    }
    
    //Правая половина всегда обрабатывается в текущем потоке
    parallel_merge_sort_thread(right_args);
    
    if (left_created){
        pthread_join(left_thread, NULL); // Ожидание потока
        pthread_mutex_lock(&mutex);
        active_threads--;
        pthread_mutex_unlock(&mutex);
    }
    
    merge(array, left, mid, right);
    free(args);
    return NULL;
}

// Основная функция параллельной сортировки слиянием
void parallel_merge_sort(int *array, int size, int max_threads) {
    SortArgs *args = malloc(sizeof(SortArgs));
    args->array = array;
    args->left = 0;
    args->right = size - 1;
    args->depth = 0;
    args->max_threads = max_threads;
    
    parallel_merge_sort_thread(args);
}

int is_sorted(int *array, int size) {
    for (int i = 1; i < size; i++) {
        if (array[i] < array[i - 1]) {
            return 0;
        }
    }
    return 1;
}

void print_array(int *array, int size) {
    printf("[");
    for (int i = 0; i < size && i < 20; i++) {
        printf("%d", array[i]);
        if (i < size - 1 && i < 19) printf(", ");
    }
    if (size > 20) printf(", ...");
    printf("]\n");
}