#ifndef PARALLEL_H
#define PARALLEL_H

#include <pthread.h>

double get_time(void);
void merge(int *array, int left, int mid, int right);
void sequential_merge_sort(int *array, int left, int right);
void parallel_merge_sort(int *array, int size, int max_threads);
int is_sorted(int *array, int size);
void print_array(int *array, int size);

#endif