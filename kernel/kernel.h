#ifndef KERNEL_H
#define KERNEL_H

#include "globals.h"

void user_input(char *input);
kernel_globals *get_kernel_globals();

#endif

    // asm volatile(
    //     "pushf;" // Push the EFLAGS value to the stack
    //     "pop %%eax;" // Pop EFLAGS to the register eax
    //     : "=r" (test) // Set the test with the value of eax
    // );


    // strfmt("%u\n", temp, 32, zaz);
    // kprint(temp);

    // asm volatile(
    //     "pushf;" // Push the EFLAGS value to the stack
    //     "pop %eax;" // Pop EFLAGS to the register eax
    //     "mov 0x00020000, %ebx;" // Set ebx register to correspond to the 17th bit
    //     "or %eax, %ebx;"
    //     "push %eax;" // Push eax register to the stack
    //     "popf" // Pop the eax register to EFLAGS value
    // );

    // asm volatile(
    //     "pushf;" // Push the EFLAGS value to the stack
    //     "pop %%eax;" // Pop EFLAGS to the register eax
    //     : "=r" (test) // Set the test with the value of eax
    // );
