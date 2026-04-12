#include "kercall.h"
#include <stdint.h>

#include "../cpu/timer.h"
#include "../drivers/display/display.h"
#include "../drivers/pcspk.h"
#include "../power/powerctl.h"
#include "kernel.h"

// --- Timer Functions ---

void kercall_timer_pit_sleep_ms(registers_t *regs) {
    uint64_t time = (uint64_t)regs->ecx << 32 | regs->ebx;
    sleep_ms(time);
}

uint64_t kercall_timer_pit_get_ms(registers_t *regs) {
    (void)regs;
    return get_tick();
}

// --- Clock functions ---

uintptr_t kercall_rtc_get_time(registers_t *regs) {
    (void)regs;
    return (uintptr_t)&get_kernel_globals()->datetime;
}

void kercall_rtc_get_date(registers_t *regs) {
    regs->eax = get_kernel_globals()->datetime.year;
    regs->ebx = get_kernel_globals()->datetime.month;
    regs->ecx = get_kernel_globals()->datetime.day;
}

void kercall_rtc_get_time_new(registers_t *regs) {
    regs->eax = get_kernel_globals()->datetime.hours;
    regs->ebx = get_kernel_globals()->datetime.minutes;
    regs->ecx = get_kernel_globals()->datetime.seconds;
    regs->edx = get_kernel_globals()->datetime.milliseconds;
}

// --- Power Control functions ---

void kercall_powerctl_reboot(registers_t *regs) {
    (void)regs;
    reboot();
}

void kercall_powerctl_halt(registers_t *regs) {
    (void)regs;
    halt();
}

// --- PC Speaker functions ---

void kercall_pcspk_beep(registers_t *regs) {
    (void)regs;
    speaker_set_frequency(440);
}

void kercall_pcspk_set_freq(registers_t *regs) {
    speaker_set_frequency(regs->ebx);
}

uint32_t kercall_pcspk_get_freq(registers_t *regs) {
    (void)regs;
    return speaker_get_freq();
}

void kercall_pcspk_stop(registers_t *regs) {
    (void)regs;
    speaker_stop();
}

// --- Display functions ---

void kercall_display_select_cur_foreground(registers_t *regs) {
    display_set_foreground(DISPLAY_COLOR(regs->ebx, regs->ecx, regs->edx));
}

void kercall_display_select_cur_background(registers_t *regs) {
    display_set_background(DISPLAY_COLOR(regs->ebx, regs->ecx, regs->edx));
}

void kercall_display_clear(registers_t *regs) {
    (void)regs;
    display_clear_screen();
}

void kercall_display_print_char_ez(registers_t *regs) {
    display_print_char_ez(regs->ebx);
}

void kercall_display_print_str(registers_t *regs) {
    display_print_str((char *)(uintptr_t)regs->ebx);
}

void kercall_display_set_cur_pos(registers_t *regs) {
    display_set_cursor_pos(regs->ebx, regs->ecx);
}
