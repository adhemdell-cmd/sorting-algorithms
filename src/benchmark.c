
#define _POSIX_C_SOURCE 200809L
#include "benchmark.h"
#include "utils.h"
#include "memtrack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#ifdef __GLIBC__
#include <malloc.h>
#endif

typedef struct {
    double wall_ms, cpu_ms;
    long   extra_kb, peak_kb;
    size_t aux_bytes;
    int    correct;
} raw_result;

static double ts_ms(clockid_t id)
{
    struct timespec t;
    clock_gettime(id, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

static long maxrss_kb(void)
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_maxrss;
}


static void child_run(sort_fn fn, const int *master, size_t n, int fd)
{
    raw_result r = {0};
#ifdef __GLIBC__

    mallopt(M_MMAP_THRESHOLD, 4096);
#endif
    int *work = copy_array(master, n);
    if (!work) _exit(2);

    memtrack_reset();
    long base_kb = maxrss_kb();
    double w0 = ts_ms(CLOCK_MONOTONIC), c0 = ts_ms(CLOCK_PROCESS_CPUTIME_ID);
    fn(work, n);
    double w1 = ts_ms(CLOCK_MONOTONIC), c1 = ts_ms(CLOCK_PROCESS_CPUTIME_ID);
    long end_kb = maxrss_kb();
    r.aux_bytes = memtrack_peak_bytes();

    r.wall_ms  = w1 - w0;
    r.cpu_ms   = c1 - c0;
    r.peak_kb  = end_kb;
    r.extra_kb = end_kb > base_kb ? end_kb - base_kb : 0;
    r.correct  = is_sorted(work, n);

    if (write(fd, &r, sizeof r) != (ssize_t)sizeof r) _exit(3);
    _exit(0);
}

static int run_once(sort_fn fn, const int *master, size_t n, raw_result *r)
{
    int fd[2];
    fflush(NULL);
    if (pipe(fd) != 0) return -1;
    pid_t pid = fork();
    if (pid < 0) { close(fd[0]); close(fd[1]); return -1; }
    if (pid == 0) { close(fd[0]); child_run(fn, master, n, fd[1]); }

    close(fd[1]);
    ssize_t got = read(fd[0], r, sizeof *r);
    close(fd[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    return (got == (ssize_t)sizeof *r && WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : -1;
}

int benchmark_sort(sort_fn fn, const int *master, size_t n, int runs, bench_result *out)
{
    if (runs < 1) runs = 1;
    memset(out, 0, sizeof *out);
    out->correct = 1;
    long max_extra = 0, max_peak = 0;
    size_t max_aux = 0;

    for (int i = 0; i < runs; i++) {
        raw_result r;
        if (run_once(fn, master, n, &r) != 0) return -1;
        out->wall_ms += r.wall_ms;
        out->cpu_ms  += r.cpu_ms;
        if (r.extra_kb > max_extra) max_extra = r.extra_kb;
        if (r.peak_kb  > max_peak)  max_peak  = r.peak_kb;
        if (r.aux_bytes > max_aux)  max_aux   = r.aux_bytes;
        if (!r.correct) out->correct = 0;
    }
    out->wall_ms    /= runs;
    out->cpu_ms     /= runs;
    out->aux_mb      = max_aux / (1024.0 * 1024.0);
    out->mem_mb      = max_extra / 1024.0;
    out->peak_rss_mb = max_peak  / 1024.0;
    out->runs        = runs;
    return 0;
}

int benchmark_default_runs(const sort_algo *algo, size_t n)
{
    return (algo->quadratic && n >= 50000) ? 1 : 5;
}
