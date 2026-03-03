#ifndef RTC_H
#define RTC_H

#include "isr.h"

void rtc_callback(registers_t *regs);
void init_rtc(uint8_t rate);
uint8_t rtc_avail();

#endif
