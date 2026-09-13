#ifndef KOR_KERNEL_H
#define KOR_KERNEL_H

#include <kor/ll/isr.h>
#include <kor/types.h>

_Noreturn void kernel_panic(u32 isr_num, i8 *exception_message,
                            registers_t *regs);

#endif // KOR_KERNEL_H
