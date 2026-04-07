#include "isr99.h"
#include "../kernel/kercall.h"
#include "isr.h"

void callback(registers_t *regs) {
    // We'll use 3000 as our base because the interface will change and with
    // time this will become legacy
    switch (regs->eax) {
    case 3000:
        kercall_timer_pit_sleep_ms(regs);
        break;
    case 3001: {

        uint64_t result = kercall_timer_pit_get_ms(regs);
        uint32_t lower = result & 0xFFFFFFFF;
        uint32_t upper = result >> 32;
        regs->eax = lower;
        regs->ebx = upper;
    } break;
    case 3002: {
        uintptr_t result = kercall_rtc_get_time(regs);
        regs->eax = result;
    } break;
    case 3003:
        kercall_powerctl_reboot(regs);
        break;
    case 3004:
        kercall_powerctl_halt(regs);
        break;
    }
}

void install_isr99() { register_interrupt_handler(153, &callback); }
