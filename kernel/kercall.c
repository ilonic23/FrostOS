#include "kercall.h"
#include <stdint.h>

#include "../cpu/timer.h"
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
