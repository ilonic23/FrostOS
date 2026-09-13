#include <kor/clock/pit.h>
#include <kor/ll/isr.h>
#include <kor/ll/port.h>
#include <kor/types.h>

static u64 tick = 0;
static u64 pit_freq = 0;

u64 pit_get_tick() { return tick; }

void pit_callback(registers_t *regs) {
    (void)regs;
    tick++;
}

#define PIT_FREQ 1193182

#define PIT_CMD 0x43
#define PIT_CH0 0x40

void pit_init(u32 freq) {
    register_interrupt_handler(IRQ0, pit_callback);

    pit_freq = freq;

    u32 divisor = PIT_FREQ / freq;
    u8 low = (u8)(divisor & 0xFF);
    u8 high = (u8)((divisor >> 8) & 0xFF);
    asm volatile("cli");
    outb(PIT_CMD, 0x36);
    outb(PIT_CH0, low);
    outb(PIT_CH0, high);
    asm volatile("sti");
}
