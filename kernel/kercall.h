/*
 * A kercall (Kernel Call) is like a system call, but made for ring 0,
 * because the kernel for now works with a flat memory model and without
 * paging. These could be used later in programs until syscalls and paging
 * wont be implemented.
 *
 * A kercall is called using 'int 99h', with a specific category of functions
 * passed to eax and then selecting a specific function from the category
 * using ebx.
 */

#ifndef KERCALL_H
#define KERCALL_H

#include "../cpu/isr.h"

void kercall_timer_pit_sleep_ms(registers_t *regs);
uint64_t kercall_timer_pit_get_ms(registers_t *regs);
uintptr_t kercall_rtc_get_time(registers_t *regs);
void kercall_powerctl_reboot(registers_t *regs);
void kercall_powerctl_halt(registers_t *regs);

#endif
