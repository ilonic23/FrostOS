#include <stdint.h>
#include "multiboot.h"
#include "../libc/stdio.h"

char *mb_get_bootloader(multiboot_info_t *mbi) {
    if (mbi->flags & (1u << 9))
        return (char *)mbi->boot_loader_name;
    return NULL;
}

// Returns lower memory amount in bytes
uint64_t mb_get_lower_mem(multiboot_info_t *mbi) {
    if (mbi->flags & (1u << 0))
        return mbi->mem_lower * 1024;
    return 0;
}

// Returns upper memory amount in bytes
uint64_t mb_get_upper_mem(multiboot_info_t *mbi) {
    if (mbi->flags & (1u << 0))
        return ((mbi->mem_upper * 1024) + 0x100000);
    return 0;
}

// Returns memory amount in bytes
uint64_t mb_get_mem(multiboot_info_t *mbi) {
    uint64_t result = 0;
    if (mbi->flags & (1u << 6))
        for (int i = 0; i < mbi->mmap_length; i += sizeof(multiboot_mmap_entry_t)) {
            multiboot_mmap_entry_t entry =
                *(multiboot_mmap_entry_t*)(mbi->mmap_addr + i);
            result += entry.len;
        }
    return result;
}
