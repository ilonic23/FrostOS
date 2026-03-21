#include "isr.h"
#include "isr99.h"
#include "../libc/stdio.h"

void callback(registers_t *regs) {
    printf("ISR99 received, EAX: %d\n", regs->eax);
}

void install_isr99() {
    register_interrupt_handler(153, &callback);
}
