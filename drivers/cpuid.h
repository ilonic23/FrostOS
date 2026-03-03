#ifndef CPUID_H
#define CPUID_H

#include <stdint.h>

extern int check_cpuid(void);

typedef struct {
    char cpu_manufacturer[13];
    char cpu_full_name[49];
    
} __attribute__ ((packed)) cpuid_t;

void get_cpu_manufacturer(char *to);
uint8_t get_cpu_human_id(char *to);

#endif
