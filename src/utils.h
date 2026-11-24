#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

typedef enum { DATA_RANDOM = 0, DATA_SORTED, DATA_REVERSE, DATA_DUPLICATES, DATA_TYPES } data_type;

extern const char *DATA_TYPE_NAMES[DATA_TYPES];

#define DEFAULT_SEED 42ULL


int  *generate_array(size_t n, data_type type, unsigned long long seed);
int  *copy_array(const int *src, size_t n);
int   is_sorted(const int *a, size_t n);
void  print_array(const char *label, const int *a, size_t n, size_t max_items);

#endif
