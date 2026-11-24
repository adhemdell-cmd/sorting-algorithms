
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "benchmark.h"
#include "sorting.h"
#include "utils.h"

static const size_t COMPARE_SIZES[] = { 1000, 5000, 10000, 50000, 100000 };
#define NUM_COMPARE_SIZES (sizeof COMPARE_SIZES / sizeof COMPARE_SIZES[0])
#define BIG_SIZE 100000
#define RESULTS_DIR "results"




static int read_line(const char *prompt, char *buf, size_t len)
{
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, (int)len, stdin)) return 0;
    if (!strchr(buf, '\n')) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }
    return 1;
}


static long read_long(const char *prompt, long min, long max)
{
    char buf[64], *end;
    if (!read_line(prompt, buf, sizeof buf)) return -1;
    errno = 0;
    long v = strtol(buf, &end, 10);
    int has_digits = (end != buf);
    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') end++;
    if (!has_digits || *end != '\0' || errno == ERANGE || v < min || v > max) return -2;
    return v;
}

static int ask_size(size_t *n)
{
    printf("\nArray size:\n  1. 1 000   2. 5 000   3. 10 000   4. 50 000   5. 100 000   6. custom\n");
    long c = read_long("Your choice: ", 1, 6);
    if (c < 0) { puts("Invalid choice."); return 0; }
    static const size_t preset[] = { 1000, 5000, 10000, 50000, 100000 };
    if (c <= 5) { *n = preset[c - 1]; return 1; }
    long v = read_long("Enter size (1 - 5 000 000): ", 1, 5000000);
    if (v < 0) { puts("Invalid size."); return 0; }
    *n = (size_t)v;
    return 1;
}

static int ask_type(data_type *t)
{
    printf("\nInput type:\n  1. random   2. already sorted   3. reverse sorted   4. many duplicates\n");
    long c = read_long("Your choice: ", 1, 4);
    if (c < 0) { puts("Invalid choice."); return 0; }
    *t = (data_type)(c - 1);
    return 1;
}



static void ensure_results_dir(void)
{
    if (mkdir(RESULTS_DIR, 0755) != 0 && errno != EEXIST)
        fprintf(stderr, "Warning: cannot create '%s': %s\n", RESULTS_DIR, strerror(errno));
}

static void print_header(void)
{
    printf("%-22s %9s %11s %10s %12s %11s %5s %3s\n",
           "Algorithm", "Size", "Time (ms)", "CPU (ms)", "Memory (MB)", "RSS+ (MB)", "Rank", "OK");
    printf("--------------------------------------------------------------------------------------------\n");
}

static void print_row(const char *name, size_t n, const bench_result *r, int rank)
{
    char rk[16];
    if (rank > 0) snprintf(rk, sizeof rk, "%d", rank); else snprintf(rk, sizeof rk, "ref");
    printf("%-22s %9zu %11.3f %10.3f %12.3f %11.3f %5s %3s\n",
           name, n, r->wall_ms, r->cpu_ms, r->aux_mb, r->mem_mb, rk, r->correct ? "yes" : "NO");
}

static void csv_header(FILE *f)
{
    fprintf(f, "input_type,size,algorithm,runs,wall_ms,cpu_ms,memory_mb,rss_growth_mb,peak_rss_mb,rank,correct\n");
}


static int compare_size(size_t n, data_type type, FILE *csv, int quiet_header)
{
    int *master = generate_array(n, type, DEFAULT_SEED);
    if (!master) { puts("Out of memory."); return -1; }

    bench_result res[5], ref;
    int rank[5];
    for (int i = 0; i < 5; i++) {
        int runs = benchmark_default_runs(&ALGOS[i], n);
        if (benchmark_sort(ALGOS[i].fn, master, n, runs, &res[i]) != 0) {
            puts("Measurement failed."); free(master); return -1;
        }
    }
    if (benchmark_sort(REFERENCE_ALGO.fn, master, n, 5, &ref) != 0) {
        puts("Reference measurement failed.");
        free(master);
        return -1;
    }

    for (int i = 0; i < 5; i++) {
        rank[i] = 1;
        for (int j = 0; j < 5; j++)
            if (res[j].wall_ms < res[i].wall_ms) rank[i]++;
    }
    if (!quiet_header) print_header();
    for (int i = 0; i < 5; i++) {
        print_row(ALGOS[i].name, n, &res[i], rank[i]);
        if (csv) fprintf(csv, "%s,%zu,%s,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%d,%d\n",
                         DATA_TYPE_NAMES[type], n, ALGOS[i].name, res[i].runs,
                         res[i].wall_ms, res[i].cpu_ms, res[i].aux_mb, res[i].mem_mb, res[i].peak_rss_mb,
                         rank[i], res[i].correct);
    }
    print_row(REFERENCE_ALGO.name, n, &ref, 0);
    if (csv) fprintf(csv, "%s,%zu,%s,%d,%.4f,%.4f,%.4f,%.4f,%.4f,0,%d\n",
                     DATA_TYPE_NAMES[type], n, "qsort (reference)", ref.runs,
                     ref.wall_ms, ref.cpu_ms, ref.aux_mb, ref.mem_mb, ref.peak_rss_mb, ref.correct);
    free(master);
    return 0;
}

static void note_runs(void)
{
    puts("(Average of 5 runs; Bubble/Selection/Insertion use 1 run when size >= 50 000 because they are very slow.)");
}



static void run_single(int idx)
{
    size_t n; data_type type;
    if (!ask_size(&n) || !ask_type(&type)) return;

    int *master = generate_array(n, type, DEFAULT_SEED);
    if (!master) { puts("Out of memory."); return; }

    const sort_algo *a = &ALGOS[idx];
    printf("\n%s on %zu %s integers (seed %llu)\n", a->name, n, DATA_TYPE_NAMES[type], DEFAULT_SEED);
    if (a->quadratic && n >= 50000) puts("This algorithm is O(n^2): it may take several seconds...");

    if (n <= 20) {
        int *show = copy_array(master, n);
        print_array("Before:", master, n, 20);
        a->fn(show, n);
        print_array("After: ", show, n, 20);
        free(show);
    }
    bench_result r;
    if (benchmark_sort(a->fn, master, n, benchmark_default_runs(a, n), &r) != 0) {
        puts("Measurement failed.");
    } else {
        printf("\n  Wall-clock time : %.3f ms\n  CPU time        : %.3f ms\n"
               "  Memory (exact)  : %.3f MB (heap allocated by the sort itself, array excluded)\n"
               "  RSS growth      : %.3f MB (getrusage ru_maxrss, ~0.25 MB resolution)\n"
               "  Process peak RSS: %.3f MB\n  Runs averaged   : %d\n  Correctly sorted: %s\n",
               r.wall_ms, r.cpu_ms, r.aux_mb, r.mem_mb, r.peak_rss_mb, r.runs, r.correct ? "yes" : "NO");
    }
    free(master);
}

static void run_compare(void)
{
    data_type type;
    if (!ask_type(&type)) return;
    ensure_results_dir();
    char path[128];
    snprintf(path, sizeof path, RESULTS_DIR "/compare_%s.csv", DATA_TYPE_NAMES[type]);
    FILE *csv = fopen(path, "w");
    if (csv) csv_header(csv);

    printf("\nComparison on %s data (seed %llu)\n", DATA_TYPE_NAMES[type], DEFAULT_SEED);
    note_runs();
    print_header();
    int failed = 0;
    for (size_t i = 0; i < NUM_COMPARE_SIZES; i++) {
        if (compare_size(COMPARE_SIZES[i], type, csv, 1) != 0) {
            failed = 1;
            break;
        }
        puts("");
        fflush(stdout);
    }
    if (csv) {
        fclose(csv);
        if (failed) printf("Incomplete results written to %s\n", path);
        else printf("Results exported to %s\n", path);
    }
    else printf("Could not write %s\n", path);
}

static void run_big(void)
{
    printf("\nBIG ARRAY EXAMPLE: %d random integers, fixed seed %llu\n", BIG_SIZE, DEFAULT_SEED);
    puts("Bubble/Selection/Insertion are O(n^2): expect several seconds each.");
    note_runs();
    print_header();
    compare_size(BIG_SIZE, DATA_RANDOM, NULL, 1);
}


static int run_full(const char *path)
{
    ensure_results_dir();
    FILE *csv = fopen(path, "w");
    if (!csv) { fprintf(stderr, "Cannot write %s: %s\n", path, strerror(errno)); return 1; }
    csv_header(csv);
    note_runs();
    int failed = 0;
    for (int t = 0; t < DATA_TYPES && !failed; t++) {
        printf("\n=== Input type: %s (seed %llu) ===\n", DATA_TYPE_NAMES[t], DEFAULT_SEED);
        print_header();
        for (size_t i = 0; i < NUM_COMPARE_SIZES; i++) {
            if (compare_size(COMPARE_SIZES[i], (data_type)t, csv, 1) != 0) {
                failed = 1;
                break;
            }
            puts("");
            fflush(stdout);
        }
    }
    fclose(csv);
    if (failed) {
        fprintf(stderr, "Benchmark failed; incomplete results remain in %s\n", path);
        return 1;
    }
    printf("All results written to %s\n", path);
    return 0;
}

static void print_menu(void)
{
    puts("\n=========== SORTING ALGORITHMS ===========");
    puts("1. Bubble Sort");
    puts("2. Selection Sort");
    puts("3. Insertion Sort");
    puts("4. Merge Sort");
    puts("5. Quick Sort");
    puts("6. Compare the five main algorithms");
    puts("7. Heap Sort (Bonus)");
    puts("8. Shell Sort (Bonus)");
    puts("9. Counting Sort (Bonus)");
    puts("10. Big array example (100 000 integers)");
    puts("0. Exit");
}

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "--full") == 0)
        return run_full(argc >= 3 ? argv[2] : RESULTS_DIR "/results.csv");
    if (argc >= 2 && strcmp(argv[1], "--big") == 0) { run_big(); return 0; }
    if (argc >= 2) {
        fprintf(stderr, "Usage: %s [--full [file.csv] | --big]\n", argv[0]);
        return 1;
    }

    for (;;) {
        print_menu();
        long c = read_long("Your choice: ", 0, 10);
        if (c == -1) { puts("\nEnd of input. Bye!"); break; }
        if (c == -2) { puts("Invalid choice, please enter a number between 0 and 10."); continue; }
        if (c == 0) { puts("Bye!"); break; }
        if (c >= 1 && c <= 5) run_single((int)c - 1);
        else if (c == 6) run_compare();
        else if (c >= 7 && c <= 9) run_single((int)c - 2);
        else if (c == 10) run_big();
    }
    return 0;
}
