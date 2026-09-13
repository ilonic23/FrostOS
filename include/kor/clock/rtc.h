#ifndef KOR_CLOCK_RTC_H
#define KOR_CLOCK_RTC_H

#include <kor/clock/clock.h>
#include <kor/types.h>

void rtc_init(u8 rate);
void rtc_read_datetime(datetime_t *dt);
void datetime_add_ms(u64 ms, datetime_t *dt);
datetime_t *rtc_get_datetime();

#endif // KOR_CLOCK_RTC_H
