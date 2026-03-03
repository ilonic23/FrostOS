#include "./isr.h"
#include "../libc/function.h"
#include "./ports.h"
#include "../drivers/clock.h"
#include "../kernel/kernel.h"

static uint8_t rtc_available = 0;
static uint8_t periodic_interrupt = 0;

void rtc_callback(registers_t *regs) {
    port_byte_out(0x70, 0x0C);  // select register C
    uint8_t status = port_byte_in(0x71);
    
    // Periodic interrupt
    if (status & 0x40) {
        periodic_interrupt = 1;
        // TODO: Add datetime update handling
        datetime_add_ms(1, &get_kernel_globals()->datetime);
    }
    // Clock update (1Hz)
    if (status & 0x10) {
        rtc_available = 1;
    }
    UNUSED(regs);
}

uint8_t rtc_avail() {
    uint8_t temp = rtc_available;
    if (rtc_available)
        rtc_available = 0;
    return temp;
}

uint8_t periodic_interr() {
    uint8_t temp = periodic_interrupt;
    if (periodic_interrupt)
        periodic_interrupt = 0;
    return temp;
}

// Example took from OSDEV: https://wiki.osdev.org/RTC#Turning_on_IRQ_8
void init_rtc(uint8_t rate) {
    rate &= 0x0F; // rate must be above 2 and not over 15 ( 6 = 1024 Hz ~1 ms)

    register_interrupt_handler(IRQ8, &rtc_callback);
    asm volatile("cli"); // disable interrupts
    port_byte_out(0x70, 0x8B);
char prevB = port_byte_in(0x71);
port_byte_out(0x70, 0x8B);
port_byte_out(0x71, prevB | 0x40);  // enable PIE
    port_byte_out(0x70, 0x8A); // select register A, disable NMI
    char prev = port_byte_in(0x71);	// read the current value of register A
    port_byte_out(0x70, 0x8A); // reset index to A
    port_byte_out(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
    asm volatile("sti");
}
