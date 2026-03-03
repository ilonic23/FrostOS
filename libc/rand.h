/*
CSPRNG (I hope so) based on PCG and RDSEED instruction
- Made by Ilonic 2025
*/

#ifndef RAND_H
#define RAND_H

#include <stdint.h>

static uint64_t state = 0x4d595df4d0f33173; // Or something seed-dependent
static uint64_t const multiplier = 6364136223846793005u;
static uint64_t const increment = 1442695040888963407u; // Or an arbitrary odd constant

static uint32_t rotr32(uint32_t x, unsigned r) {
    return x >> r | x << (-r & 31); 
}

uint32_t pcg32(void) { 
    uint64_t x = state;
    unsigned count = (unsigned)(x >> 59); // 59 = 64 - 5 
    state = x * multiplier + increment;
    x ^= x >> 18; // 18 = (64 - 27)/2 
    return rotr32((uint32_t)(x >> 27), count); // 27 = 32 - 5 
}

void pcg32_init(uint64_t seed) {
    state = seed + increment; 
    (void)pcg32(); 
}

void pcg32_self_init() {
    uint32_t seed = 1234;
    uint8_t ok;

    // for (int i = 0; i < 100; ++i) {
    //     asm volatile(
    //         "rdrand %0\n\t"
    //         "setc %1"
    //         : "=r"(seed), "=qm"(ok)
    //     );

    //     if (ok) {
    //         state = (uint64_t)seed + increment;
    //         (void)pcg32();
    //     }
    // }
    state = (uint64_t)seed + increment;
    (void)pcg32();
}

#endif