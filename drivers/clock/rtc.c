#include <kor/clock/clock.h>
#include <kor/clock/rtc.h>
#include <kor/ll/isr.h>
#include <kor/ll/port.h>
#include <kor/types.h>

static u8 rtc_available = 0;
static datetime_t time;

datetime_t *rtc_get_datetime() { return &time; }

#define CMOS1 0x70
#define CMOS2 0x71

#define STATUS_A 0x0A
#define STATUS_B 0x0B
#define STATUS_C 0x0C

#define DISABLE_NMI 0x80
#define ENABLE_PIE 0x40

enum CMOS_Registers {
    SECONDS = 0x00,
    MINUTES = 0x02,
    HOURS = 0x04,
    WEEKDAY = 0x06, // Sunday = 1
    MONTHDAY = 0x07,
    MONTH = 0x08,
    YEAR = 0x09,
    CENTURY = 0x32,
};

void rtc_callback(registers_t *regs) {
    (void)regs;
    outb(CMOS1, STATUS_C);
    u8 status = inb(CMOS2);

    // Periodic interrupt
    if (status & 0x40) {
        datetime_add_ms(1, &time);
    }

    if (status & 0x10) {
        rtc_available = 1;
    }
}

static u8 rtc_avail() {
    u8 tmp = rtc_available;
    if (rtc_available)
        rtc_available = 0;
    return tmp;
}

void rtc_init(u8 rate) {
    register_interrupt_handler(IRQ8, &rtc_callback);
    rate &= 0x0F;

    asm volatile("cli");
    outb(CMOS1, DISABLE_NMI | STATUS_B);
    i8 prevB = inb(CMOS2);
    outb(CMOS1, DISABLE_NMI | STATUS_B);
    outb(CMOS2, prevB | ENABLE_PIE);
    outb(CMOS1, DISABLE_NMI | STATUS_A);
    i8 prev = inb(CMOS2);
    outb(CMOS1, DISABLE_NMI | STATUS_A);
    outb(CMOS2, (prev & 0xF0) | rate);
    asm volatile("sti");
    rtc_read_datetime(&time);
}

static inline u8 rtc_read(u8 reg) {
    outb(CMOS1, DISABLE_NMI | reg);
    return inb(CMOS2);
}

static inline u8 bcd_to_bin(u8 bcd) {
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0xf);
}

void rtc_read_datetime(datetime_t *dt) {
    u8 seconds, minutes, hours, day, month, year, century = 0;
    u8 registryB;

    while (!rtc_avail())
        asm volatile("hlt");

    u8 second1, second2; // To get stable value

    do { // For more info:
         // https://wiki.osdev.org/CMOS#Getting_Current_Date_andime_from_RTC
        second1 = rtc_read(SECONDS);
        minutes = rtc_read(MINUTES);
        hours = rtc_read(HOURS);
        day = rtc_read(MONTHDAY);
        month = rtc_read(MONTH);
        year = rtc_read(YEAR);
        century = rtc_read(CENTURY);
        second2 = rtc_read(SECONDS);
    } while (second1 != second2);

    registryB = rtc_read(STATUS_B);

    if (!(registryB & 0x04)) { // If BCD used
        seconds = bcd_to_bin(second1);
        minutes = bcd_to_bin(minutes);

        u8 h = bcd_to_bin(
            hours & 0x7F); // Hours can be in AM/PM mode (7th bit as PM flag)
        if (!(registryB & 0x02)) { // AM/PM
            if (hours & 0x80) {    // PM
                if (h != 12)
                    h += 12;
            } else {
                if (h == 12)
                    h = 0;
            }
        }

        hours = h;
        day = bcd_to_bin(day);
        month = bcd_to_bin(month);
        year = bcd_to_bin(year);
        if (century)
            century = bcd_to_bin(century);
    } else {
        seconds = second1;

        // Hours can be in AM/PM mode (7th bit as PM flag)
        if (!(registryB & 0x02)) { // AM/PM
            u8 h = hours & 0x7F;
            if (hours & 0x80) { // PM
                if (h != 12)
                    h += 12;
            } else {
                if (h == 12)
                    h = 0;
            }
            hours = h;
        }
    }

    u16 full_year;
    if (century)
        full_year = (u16)century * 100u + (u16)year;
    else
        full_year = (year < 70 ? 2000u + year : 1900u + year);

    dt->year = full_year;
    dt->month = month;
    dt->day = day;
    dt->hours = hours;
    dt->minutes = minutes;
    dt->seconds = seconds;
}

// non-leap year
static const u8 days_in_month[13] = {0,  31, 28, 31, 30, 31, 30,
                                     31, 31, 30, 31, 30, 31};

static u8 is_leap_year(unsigned int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static u8 get_days_in_month(unsigned int month, unsigned int year) {
    if (month == 2 && is_leap_year(year))
        return 29;
    return days_in_month[month];
}

void datetime_add_ms(u64 ms, datetime_t *dt) {
    u64 total_ms = dt->milliseconds + ms;

    dt->milliseconds = total_ms % 1000;
    u64 carry_s = total_ms / 1000;
    if (carry_s == 0)
        return;

    u64 total_s = dt->seconds + carry_s;
    dt->seconds = total_s % 60;
    u64 carry_m = total_s / 60;
    if (carry_m == 0)
        return;

    u64 total_m = dt->minutes + carry_m;
    dt->minutes = total_m % 60;
    u64 carry_h = total_m / 60;
    if (carry_h == 0)
        return;

    u64 total_h = dt->hours + carry_h;
    dt->hours = total_h % 24;
    u64 carry_d = total_h / 24;
    if (carry_d == 0)
        return;

    while (carry_d > 0) {
        u8 dim = get_days_in_month(dt->month, dt->year);
        u64 remaining = dim - dt->day;

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
