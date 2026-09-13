#ifndef KOR_CLOCK_PIT_H
#define KOR_CLOCK_PIT_H

#include <kor/types.h>

u64 pit_get_tick();
void pit_init(u32 freq);

#endif
