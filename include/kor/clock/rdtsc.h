#ifndef KOR_CLOCK_RDTSC_H
#define KOR_CLOCK_RDTSC_H

#include <kor/types.h>

static inline u64 rdtsc() {
    u32 low, high;
    asm volatile("rdtsc;" : "=a"(low), "=d"(high) : : "memory");
    return ((u64)high << 32 | low);
}

#endif
