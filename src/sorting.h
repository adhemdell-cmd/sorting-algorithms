#ifndef SORTING_H
#define SORTING_H

#include <stddef.h>


typedef void (*sort_fn)(int *a, size_t n);

typedef struct {
    const char *name;
    sort_fn     fn;
    int         quadratic;
} sort_algo;

#define NUM_ALGOS 8

void bubble_sort(int *a, size_t n);
void selection_sort(int *a, size_t n);
void insertion_sort(int *a, size_t n);
void merge_sort(int *a, size_t n);
void quick_sort(int *a, size_t n);
void heap_sort(int *a, size_t n);
void shell_sort(int *a, size_t n);
void counting_sort(int *a, size_t n);


void builtin_qsort(int *a, size_t n);


extern const sort_algo ALGOS[NUM_ALGOS];
extern const sort_algo REFERENCE_ALGO;

#endif
