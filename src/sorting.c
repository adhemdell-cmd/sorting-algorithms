
#include "sorting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void swap_int(int *x, int *y) { int t = *x; *x = *y; *y = t; }


void bubble_sort(int *a, size_t n)
{
    if (n < 2) return;
    for (size_t pass = 0; pass + 1 < n; pass++) {
        int swapped = 0;
        for (size_t j = 0; j + 1 < n - pass; j++) {
            if (a[j] > a[j + 1]) {
                swap_int(&a[j], &a[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}


void selection_sort(int *a, size_t n)
{
    if (n < 2) return;
    for (size_t i = 0; i + 1 < n; i++) {
        size_t min = i;
        for (size_t j = i + 1; j < n; j++)
            if (a[j] < a[min]) min = j;
        if (min != i) swap_int(&a[i], &a[min]);
    }
}


void insertion_sort(int *a, size_t n)
{
    for (size_t i = 1; i < n; i++) {
        int key = a[i];
        size_t j = i;
        while (j > 0 && a[j - 1] > key) {
            a[j] = a[j - 1];
            j--;
        }
        a[j] = key;
    }
}


static void merge_rec(int *a, int *tmp, size_t lo, size_t hi)
{
    if (hi - lo < 2) return;
    size_t mid = lo + (hi - lo) / 2;
    merge_rec(a, tmp, lo, mid);
    merge_rec(a, tmp, mid, hi);

    size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
        tmp[k++] = (a[j] < a[i]) ? a[j++] : a[i++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi)  tmp[k++] = a[j++];
    memcpy(a + lo, tmp + lo, (hi - lo) * sizeof(int));
}

void merge_sort(int *a, size_t n)
{
    if (n < 2) return;
    int *tmp = malloc(n * sizeof(int));
    if (!tmp) { fprintf(stderr, "merge_sort: out of memory\n"); exit(EXIT_FAILURE); }
    merge_rec(a, tmp, 0, n);
    free(tmp);
}


static int median3(int x, int y, int z)
{
    if (x < y) {
        if (y < z) return y;
        return (x < z) ? z : x;
    }
    if (x < z) return x;
    return (y < z) ? z : y;
}

static void quick_rec(int *a, long lo, long hi)
{
    while (lo < hi) {
        int pivot = median3(a[lo], a[lo + (hi - lo) / 2], a[hi]);
        long i = lo - 1, j = hi + 1;
        for (;;) {
            do { i++; } while (a[i] < pivot);
            do { j--; } while (a[j] > pivot);
            if (i >= j) break;
            swap_int(&a[i], &a[j]);
        }

        if (j - lo < hi - j) {
            quick_rec(a, lo, j);
            lo = j + 1;
        } else {
            quick_rec(a, j + 1, hi);
            hi = j;
        }
    }
}

void quick_sort(int *a, size_t n)
{
    if (n < 2) return;
    quick_rec(a, 0, (long)n - 1);
}

static void sift_down(int *a, size_t start, size_t end)
{
    size_t root = start;
    while (root * 2 + 1 <= end) {
        size_t child = root * 2 + 1;
        if (child + 1 <= end && a[child] < a[child + 1]) child++;
        if (a[root] >= a[child]) return;
        swap_int(&a[root], &a[child]);
        root = child;
    }
}

void heap_sort(int *a, size_t n)
{
    if (n < 2) return;
    for (size_t i = n / 2; i > 0; i--) sift_down(a, i - 1, n - 1);
    for (size_t end = n - 1; end > 0; end--) {
        swap_int(&a[0], &a[end]);
        sift_down(a, 0, end - 1);
    }
}

void shell_sort(int *a, size_t n)
{
    for (size_t gap = n / 2; gap > 0; gap /= 2)
        for (size_t i = gap; i < n; i++) {
            int value = a[i];
            size_t j = i;
            while (j >= gap && a[j - gap] > value) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = value;
        }
}

void counting_sort(int *a, size_t n)
{
    if (n < 2) return;
    int min = a[0], max = a[0];
    for (size_t i = 1; i < n; i++) {
        if (a[i] < min) min = a[i];
        if (a[i] > max) max = a[i];
    }
    unsigned long long range = (unsigned long long)((long long)max - (long long)min) + 1;
    if (range > 10000000ULL) {
        quick_sort(a, n);
        return;
    }
    size_t *counts = malloc((size_t)range * sizeof(*counts));
    if (!counts) {
        quick_sort(a, n);
        return;
    }
    memset(counts, 0, (size_t)range * sizeof(*counts));
    for (size_t i = 0; i < n; i++) counts[(size_t)((long long)a[i] - min)]++;
    size_t k = 0;
    for (unsigned long long i = 0; i < range; i++)
        while (counts[i]--) a[k++] = (int)((long long)min + (long long)i);
    free(counts);
}


static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x, b = *(const int *)y;
    return (a > b) - (a < b);
}

void builtin_qsort(int *a, size_t n) { qsort(a, n, sizeof(int), cmp_int); }

const sort_algo ALGOS[NUM_ALGOS] = {
    { "Bubble Sort",    bubble_sort,    1 },
    { "Selection Sort", selection_sort, 1 },
    { "Insertion Sort", insertion_sort, 1 },
    { "Merge Sort",     merge_sort,     0 },
    { "Quick Sort",     quick_sort,     0 },
    { "Heap Sort",      heap_sort,      0 },
    { "Shell Sort",     shell_sort,     0 },
    { "Counting Sort",  counting_sort,  0 },
};

const sort_algo REFERENCE_ALGO = { "qsort() (reference)", builtin_qsort, 0 };
