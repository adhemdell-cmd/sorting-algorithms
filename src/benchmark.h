#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>
#include "sorting.h"

typedef struct {
    double wall_ms;
    double cpu_ms;
    double aux_mb;
    double mem_mb;
    double peak_rss_mb;
    int    runs;
    int    correct;
} bench_result;


int benchmark_sort(sort_fn fn, const int *master, size_t n, int runs, bench_result *out);


int benchmark_default_runs(const sort_algo *algo, size_t n);

#endif
