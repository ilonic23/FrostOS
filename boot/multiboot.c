#include <kor/boot/multiboot.h>
#include <kor/types.h>

u64 mb_get_mem(multiboot_info_t *mbi) {
    u64 result = 0;
    if (mbi->flags & (1u << 6))
        for (u32 i = 0; i < mbi->mmap_length;
             i += sizeof(multiboot_mmap_entry_t)) {
            multiboot_mmap_entry_t entry =
                *(multiboot_mmap_entry_t *)(usize)(mbi->mmap_addr + i);
            result += entry.len;
        }
    return result;
}
