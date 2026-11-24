
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/sorting.h"
#include "../src/utils.h"

static int failures = 0, checks = 0;

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x, b = *(const int *)y;
    return (a > b) - (a < b);
}

static void check(const sort_algo *algo, const int *src, size_t n, const char *what)
{
    int *got = copy_array(src, n), *exp = copy_array(src, n);
    algo->fn(got, n);
    qsort(exp, n, sizeof(int), cmp_int);
    checks++;
    if (memcmp(got, exp, n * sizeof(int)) != 0) {
        failures++;
        printf("FAIL: %-15s %-28s n=%zu\n", algo->name, what, n);
    }
    free(got); free(exp);
}

int main(void)
{
    const sort_algo *all[NUM_ALGOS + 1];
    for (int i = 0; i < NUM_ALGOS; i++) all[i] = &ALGOS[i];
    all[NUM_ALGOS] = &REFERENCE_ALGO;

    const int tiny[][6] = {
        {0}, {5}, {2, 1}, {1, 2}, {3, 3}, {3, 1, 2}, {1, 1, 1, 1, 1, 1},
        {-5, 10, -5, 0, 7, -1}, {2147483647, -2147483647 - 1, 0, 5, 5, -3}
    };
    const size_t tiny_n[] = { 0, 1, 2, 2, 2, 3, 6, 6, 6 };

    for (int a = 0; a <= NUM_ALGOS; a++) {
        check(all[a], NULL, 0, "empty array");
        for (size_t t = 0; t < sizeof tiny_n / sizeof tiny_n[0]; t++)
            check(all[a], tiny[t], tiny_n[t], "small hand-made case");

        const size_t sizes[] = { 3, 10, 100, 1000, 5000 };
        for (size_t s = 0; s < sizeof sizes / sizeof sizes[0]; s++)
            for (int t = 0; t < DATA_TYPES; t++)
                for (unsigned long long seed = 1; seed <= 3; seed++) {
                    int *d = generate_array(sizes[s], (data_type)t, seed);
                    check(all[a], d, sizes[s], DATA_TYPE_NAMES[t]);
                    free(d);
                }
    }


    for (int a = 3; a < NUM_ALGOS; a++)
        for (int t = 0; t < DATA_TYPES; t++) {
            int *d = generate_array(200000, (data_type)t, DEFAULT_SEED);
            check(all[a], d, 200000, "large input (recursion depth)");
            free(d);
        }

    printf("%d checks, %d failure(s)\n", checks, failures);
    puts(failures ? "TESTS FAILED" : "ALL TESTS PASSED");
    return failures ? 1 : 0;
}
