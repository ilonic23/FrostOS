#ifndef KOR_LL_IDT_H
#define KOR_LL_IDT_H

#include <kor/types.h>

typedef struct __attribute__((packed)) {
    u16 offset_lo;
    u16 sel;
    u8 always0;
    u8 flags;
    u16 offset_hi;
} idt_gate_t;

typedef struct __attribute__((packed)) {
    u16 size;
    usize offset;
} idt_reg_t;

#define IDT_ENTRIES 256
extern idt_gate_t idt[IDT_ENTRIES];
extern idt_reg_t idt_reg;

void set_idt_gate(int n, usize handler);
void set_idt();

#endif // KOR_LL_IDT_H
