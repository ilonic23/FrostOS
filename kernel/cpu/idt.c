#include <kor/ll/idt.h>
#include <kor/types.h>

idt_gate_t idt[IDT_ENTRIES];
idt_reg_t idt_reg;

#define KERNEL_CS 0x08

void set_idt_gate(int n, usize handler) {
    idt[n].offset_lo = (u16)(handler & 0xFFFF);
    idt[n].sel = KERNEL_CS;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
    idt[n].offset_hi = (u16)((handler >> 16) & 0xFFFF);
}

void set_idt() {
    idt_reg.offset = (usize)&idt;
    idt_reg.size = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    asm volatile("lidtl (%0)" : : "r"(&idt_reg));
}
