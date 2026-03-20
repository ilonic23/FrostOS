#ifndef RAND_H
#define RAND_H

#include <stdint.h>

uint32_t pcg32(void);
void pcg32_init(uint64_t seed);
uint32_t pcg32_lim(uint32_t low, uint32_t high);
#endif
