
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *DATA_TYPE_NAMES[DATA_TYPES] = {
    "random", "sorted", "reverse", "duplicates"
};


static unsigned long long next_rand(unsigned long long *s)
{
    *s ^= *s >> 12;
    *s ^= *s << 25;
    *s ^= *s >> 27;
    return *s * 2685821657736338717ULL;
}

int *generate_array(size_t n, data_type type, unsigned long long seed)
{
    int *a = malloc((n ? n : 1) * sizeof(int));
    if (!a) return NULL;
    unsigned long long state = seed ? seed : DEFAULT_SEED;

    for (size_t i = 0; i < n; i++) {
        switch (type) {
        case DATA_SORTED:     a[i] = (int)i;                                   break;
        case DATA_REVERSE:    a[i] = (int)(n - i);                             break;
        case DATA_DUPLICATES: a[i] = (int)(next_rand(&state) % 100);           break;
        case DATA_RANDOM:
        default:              a[i] = (int)(next_rand(&state) % 1000000);       break;
        }
    }
    return a;
}

int *copy_array(const int *src, size_t n)
{
    int *c = malloc((n ? n : 1) * sizeof(int));
    if (c && n) memcpy(c, src, n * sizeof(int));
    return c;
}

int is_sorted(const int *a, size_t n)
{
    for (size_t i = 1; i < n; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

void print_array(const char *label, const int *a, size_t n, size_t max_items)
{
    printf("%s [", label);
    size_t shown = n < max_items ? n : max_items;
    for (size_t i = 0; i < shown; i++) printf(i ? ", %d" : "%d", a[i]);
    if (shown < n) printf(", ... (%zu more)", n - shown);
    printf("]\n");
}
