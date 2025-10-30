#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "parallel.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Использование: %s <размер_массива> <макс_потоков> [--print]\n", argv[0]);
        return 1;
    }
    
    int size = atoi(argv[1]);
    int max_threads = atoi(argv[2]);
    int print_array_flag = (argc > 3 && strcmp(argv[3], "--print") == 0);
    
    if (size <= 0 || max_threads <= 0) {
        printf("Ошибка: размер массива и количество потоков должны быть положительными\n");
        return 1;
    }
    
    int *array = malloc(size * sizeof(int));
    int *array_seq = malloc(size * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 1000;
        array_seq[i] = array[i];
    }
    
    if (print_array_flag) {
        printf("Исходный массив: ");
        print_array(array, size);
    }
    
    printf("Размер массива: %d\n", size);
    printf("Максимальное количество потоков: %d\n", max_threads);
    printf("Количество ядер в системе: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    
    double start_time = get_time();
    parallel_merge_sort(array, size, max_threads);
    double parallel_time = get_time() - start_time;
    
    start_time = get_time();
    sequential_merge_sort(array_seq, 0, size - 1);
    double sequential_time = get_time() - start_time;
    
    if (!is_sorted(array, size)) {
        printf("ОШИБКА: Массив не отсортирован после параллельной сортировки!\n");
        free(array);
        free(array_seq);
        return 1;
    }
    
    if (!is_sorted(array_seq, size)) {
        printf("ОШИБКА: Массив не отсортирован после последовательной сортировки!\n");
        free(array);
        free(array_seq);
        return 1;
    }
    
    if (print_array_flag) {
        printf("Отсортированный массив: ");
        print_array(array, size);
    }
    
    double speedup = sequential_time / parallel_time;
    double efficiency = speedup / max_threads;
    
    printf("\n=== РЕЗУЛЬТАТЫ ===\n");
    printf("Время последовательной сортировки: %.6f сек\n", sequential_time);
    printf("Время параллельной сортировки: %.6f сек\n", parallel_time);
    printf("Ускорение (Speedup): %.4f\n", speedup);
    printf("Эффективность (Efficiency): %.4f\n", efficiency);
    
    printf("\n=== ИССЛЕДОВАНИЕ ЗАВИСИМОСТИ ОТ КОЛИЧЕСТВА ПОТОКОВ ===\n");
    printf("Потоки\tВремя(с)\tУскорение\tЭффективность\n");
    printf("------------------------------------------------\n");
    
    for (int threads = 1; threads <= max_threads; threads++) {
        memcpy(array, array_seq, size * sizeof(int));
        
        start_time = get_time();
        parallel_merge_sort(array, size, threads);
        double time = get_time() - start_time;
        
        double sp = sequential_time / time;
        double eff = sp / threads;
        
        printf("%d\t%.6f\t%.4f\t%.4f\n", threads, time, sp, eff);
    }
    
    free(array);
    free(array_seq);
    
    return 0;
}