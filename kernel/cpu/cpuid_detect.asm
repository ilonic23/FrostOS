check_cpuid:
    pushfd                      ; Save original EFLAGS
    pushfd                      ; Copy of original EFLAGS
    xor dword [esp], 0x00200000 ; Flip ID bit in the stored copy
    popfd                       ; Load modified flags
    pushfd                      ; Push modified EFLAGS
    pop eax                     ; eax = modified flags
    xor eax, [esp]              ; eax = bits that changed
    popfd                       ; Restore original EFLAGS
    and eax, 0x00200000         ; Mask only ID bit
    ret
