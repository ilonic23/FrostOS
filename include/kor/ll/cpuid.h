#ifndef KOR_LL_CPUID_H
#define KOR_LL_CPUID_H

#include <kor/types.h>

extern int check_cpuid();
typedef struct __attribute__((packed)) {
    char cpu_manufacturer[13];
    char cpu_full_name[49];
} cpuid_t;

void get_cpu_manufacturer(char *to);
u8 get_cpu_human_id(char *to);

#endif
