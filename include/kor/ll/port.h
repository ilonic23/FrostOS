#ifndef KOR_LL_PORT_H
#define KOR_LL_PORT_H

#include <kor/types.h>

static inline u8 inb(u16 port) {
    u8 res;
    asm volatile("in %%dx, %%al" : "=a"(res) : "d"(port));
    return res;
}

static inline u16 inw(u16 port) {
    u16 res;
    asm volatile("in %%dx, %%ax" : "=a"(res) : "d"(port));
    return res;
}

static inline u32 inl(u16 port) {
    u32 res;
    asm volatile("inl %1, %0" : "=a"(res) : "Nd"(port));
    return res;
}

static inline void outb(u16 port, u8 val) {
    asm volatile("out %%al, %%dx" : : "a"(val), "d"(port));
}

static inline void outw(u16 port, u16 val) {
    asm volatile("out %%ax, %%dx" : : "a"(val), "d"(port));
}

static inline void outl(u16 port, u32 val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

#endif // KOR_LL_PORT_H
