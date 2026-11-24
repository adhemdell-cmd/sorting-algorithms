
#include "memtrack.h"

#include <stdlib.h>

#define HDR 16

static size_t current_bytes, peak_bytes;

void   *__real_malloc(size_t size);
void    __real_free(void *ptr);

void *__wrap_malloc(size_t size)
{
    unsigned char *p = __real_malloc(size + HDR);
    if (!p) return NULL;
    *(size_t *)p = size;
    current_bytes += size;
    if (current_bytes > peak_bytes) peak_bytes = current_bytes;
    return p + HDR;
}

void __wrap_free(void *ptr)
{
    if (!ptr) return;
    unsigned char *p = (unsigned char *)ptr - HDR;
    current_bytes -= *(size_t *)p;
    __real_free(p);
}

void   memtrack_reset(void)       { current_bytes = 0; peak_bytes = 0; }
size_t memtrack_peak_bytes(void)  { return peak_bytes; }
