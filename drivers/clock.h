// For more info:  https://wiki.osdev.org/CMOS#Format_of_Bytes

#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71
#define NMI_DISABLE_BIT 0x80

enum CMOS_Registers {
	SECONDS = 0x00,
	MINUTES = 0x02,
	HOURS = 0x04,
	WEEKDAY = 0x06, // Sunday = 1
	MONTHDAY = 0x07,
	MONTH = 0x08,
	YEAR = 0x09,
	CENTURY = 0x32,
	STATUS_A = 0x0A,
	STATUS_B = 0x0B
};

typedef struct {
	unsigned int year : 12;
	unsigned int month : 4;
	unsigned int day : 5;
	unsigned int hours : 5;
	unsigned int minutes: 6;
	unsigned int seconds : 6;
	unsigned int milliseconds : 10;
} __attribute__ ((packed)) datetime_t;

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} datetime;

void rtc_read_datetime(datetime_t *dt);
void datetime_add_ms(uint64_t ms, datetime_t *dt);

#endif
