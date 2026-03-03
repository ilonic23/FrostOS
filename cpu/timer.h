#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void init_timer(uint32_t freq);
uint64_t get_tick();
void sleep_ms(uint64_t time_ms);

#endif
