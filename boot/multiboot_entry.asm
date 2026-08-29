section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002               ; Magic
    dd 0x00000007               ; Flags, 7 - VBE, 3 - VGA

    dd -(0x1BADB002 + 0x00000007) ; Checksum
    times 5 dd 0                ; Address fields
    dd 0                        ; VBE mode
    dd 640                      ; width
    dd 480                      ; height
    dd 16                       ; bpp

section .text
global _start
extern kernel_main

; Include the GDT definition directly here so the labels are accessible
%include "boot/gdt.asm"

_start:
    cli                         ; Disable interrupts

    ; 1. Load the GDT descriptor
    lgdt [gdt_descriptor]

    ; 2. Perform a far jump to the code segment. 
    ; This is REQUIRED to reload the CS register with 0x08.
    jmp CODE_SEG:.reload_segments

.reload_segments:
    mov ecx, eax ; preserve the eax before modifying
    ; 3. Update all data segment registers to 0x10
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 4. Set up the stack
    mov esp, stack_top
    mov ebp, esp
    
    ; 5. Verify multiboot magic (optional but good practice)
    cmp ecx, 0x2BADB002
    ; and edx, 0xFFFF0000     ; Mask out the lower 16 bits
    ; cmp edx, 0x2BAD0000     ; Check if high word is the Multiboot magic
    jnz .hang               ; If not 2BAD, it's not multiboot

    ; 6. Push multiboot parameters and call C kernel
    push ebx                    ; Multiboot info pointer
    push ecx                    ; Multiboot magic
    call kernel_main
    
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
