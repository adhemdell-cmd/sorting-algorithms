#ifndef MEMTRACK_H
#define MEMTRACK_H

#include <stddef.h>


void   memtrack_reset(void);
size_t memtrack_peak_bytes(void);

#endif
