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
void kercall_pcspk_beep(registers_t *regs);
void kercall_pcspk_set_freq(registers_t *regs);
uint32_t kercall_pcspk_get_freq(registers_t *regs);
void kercall_pcspk_stop(registers_t *regs);
void kercall_display_select_cur_foreground(registers_t *regs);
void kercall_display_select_cur_background(registers_t *regs);
void kercall_display_clear(registers_t *regs);
void kercall_display_print_char_ez(registers_t *regs);
void kercall_display_print_str(registers_t *regs);
void kercall_display_set_cur_pos(registers_t *regs);
#endif
