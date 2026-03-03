#include "clock.h"
#include "../cpu/ports.h"
#include "../cpu/rtc.h"
#include <stdint.h>

uint8_t cmos_read(uint8_t reg) {
  port_byte_out(CMOS_ADDR, NMI_DISABLE_BIT | reg);
  return port_byte_in(CMOS_DATA);
}

// Deprecated
/*uint32_t rtc_updating() {
  return (cmos_read(0x0A) & 0x80) != 0;
}*/

uint8_t bcd_to_bin(uint8_t bcd) {
  return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0xf);
}

void rtc_read_datetime(datetime_t *dt) {
  uint8_t seconds, minutes, hours, day, month, year, century = 0;
  uint8_t registryB;

  while(!rtc_avail()) asm volatile("hlt");

  uint8_t second1, second2; // To get stable value

  do { // For more info: https://wiki.osdev.org/CMOS#Getting_Current_Date_and_Time_from_RTC
    second1 = cmos_read(SECONDS);
    minutes = cmos_read(MINUTES);
    hours = cmos_read(HOURS);
    day = cmos_read(MONTHDAY);
    month = cmos_read(MONTH);
    year = cmos_read(YEAR);
    century = cmos_read(CENTURY);
    second2 = cmos_read(SECONDS);
  } while (second1 != second2);

  registryB = cmos_read(STATUS_B);

  if (!(registryB & 0x04)) { // If BCD used
    seconds = bcd_to_bin(second1);
    minutes = bcd_to_bin(minutes);
    
    uint8_t h = bcd_to_bin(hours & 0x7F); // Hours can be in AM/PM mode (7th bit as PM flag)
    if (!(registryB & 0x02)) { // AM/PM
      if (hours & 0x80) { // PM 
        if (h != 12) h += 12;
      } else {
        if (h == 12) h= 0;
      }
    }

    hours = h;
    day = bcd_to_bin(day);
    month = bcd_to_bin(month);
    year = bcd_to_bin(year);
    if (century) century = bcd_to_bin(century);
  } else {
    seconds = second1;

    // Hours can be in AM/PM mode (7th bit as PM flag)
    if (!(registryB & 0x02)) { // AM/PM
      uint8_t h = hours & 0x7F;
      if (hours & 0x80) { // PM
        if (h != 12) h += 12;
      } else {
        if (h == 12) h= 0;
      }
      hours = h;
    }
  }

  uint16_t full_year;
  if (century) full_year = (uint16_t)century * 100u + (uint16_t)year;
  else full_year = (year < 70 ? 2000u + year : 1900u + year);

  dt->year = full_year;
  dt->month = month;
  dt->day = day;
  dt->hours = hours;
  dt->minutes = minutes;
  dt->seconds = seconds;
}

// Days per month (index 1–12), non-leap year
static const uint8_t days_in_month[13] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static uint8_t is_leap_year(unsigned int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static uint8_t get_days_in_month(unsigned int month, unsigned int year) {
    if (month == 2 && is_leap_year(year))
        return 29;
    return days_in_month[month];
}

void datetime_add_ms(uint64_t ms, datetime_t *dt) {
    uint64_t total_ms = dt->milliseconds + ms;

    dt->milliseconds = total_ms % 1000;
    uint64_t carry_s = total_ms / 1000;
    if (carry_s == 0) return;

    uint64_t total_s = dt->seconds + carry_s;
    dt->seconds = total_s % 60;
    uint64_t carry_m = total_s / 60;
    if (carry_m == 0) return;

    uint64_t total_m = dt->minutes + carry_m;
    dt->minutes = total_m % 60;
    uint64_t carry_h = total_m / 60;
    if (carry_h == 0) return;

    uint64_t total_h = dt->hours + carry_h;
    dt->hours = total_h % 24;
    uint64_t carry_d = total_h / 24;
    if (carry_d == 0) return;

    while (carry_d > 0) {
        uint8_t dim = get_days_in_month(dt->month, dt->year);
        uint64_t remaining = dim - dt->day;

        if (carry_d <= remaining) {
            dt->day += carry_d;
            carry_d = 0;
        } else {
            carry_d -= remaining + 1;
            dt->day = 1;
            dt->month++;
            if (dt->month > 12) {
                dt->month = 1;
                dt->year++;
            }
        }
    }
}
