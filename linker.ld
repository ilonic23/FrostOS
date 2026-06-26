ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 0x100000;

    kernel_start = .;

    /* Multiboot header MUST come first */
    .multiboot ALIGN(4) : {
        *(.multiboot)
    }

    .text ALIGN(4K) : {
        text_start = .;
        *(.text)
        text_end = .;
    }

    .rodata ALIGN(4K) : {
        rodata_start = .;
        *(.rodata)
        rodata_end = .;
    }

    .data ALIGN(4K) : {
        data_start = .;
        *(.data)
        data_end = .;
    }

    .bss ALIGN(4K) : {
        bss_start = .;
        *(COMMON)
        *(.bss)
        bss_end = .;
    }

    kernel_end = .;
}
