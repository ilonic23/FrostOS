#ifndef KOR_BOOT_MULTIBOOT_H
#define KOR_BOOT_MULTIBOOT_H

#include <kor/types.h>

typedef struct __attribute__((packed)) {
    u32 flags;

    // flags bit 0
    u32 mem_lower;
    u32 mem_upper;

    // flags bit 1
    u32 boot_device;

    // flags bit 2
    u32 cmdline;

    // flags bit 3
    u32 mods_count;
    u32 mods_addr;

    // flags bits 4/5 (symbols)
    u32 syms[4];

    // flags bit 6 (memory map)
    u32 mmap_length;
    u32 mmap_addr;

    // flags bit 7
    u32 drives_length;
    u32 drives_addr;

    // flags bit 8
    u32 config_table;

    // flags bit 9
    u32 boot_loader_name;

    // flags bit 10
    u32 apm_table;

    // flags bit 11 -- VBE info
    u32 vbe_control_info;  // physical addr of VbeInfoBlock
    u32 vbe_mode_info;     // physical addr of ModeInfoBlock
    u16 vbe_mode;          // current VBE mode number
    u16 vbe_interface_seg; // PMI: real-mode segment
    u16 vbe_interface_off; // PMI: offset within segment
    u16 vbe_interface_len; // PMI: length in bytes

    // flags bit 12 -- Framebuffer
    u64 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8 framebuffer_bpp;
    u8 framebuffer_type;
    // color_info
    u16 garbage; // For some reason there's an offset by
                 // two bytes that's missing and the
                 // `garbage` value is here to fix it.
                 // These two bytes aren't specified
                 // anywhere. In Russian, fixes like this
                 // are named "Костыль", a crutch.
    u8 framebuffer_red_field_position;
    u8 framebuffer_red_mask_size;
    u8 framebuffer_green_field_position;
    u8 framebuffer_green_mask_size;
    u8 framebuffer_blue_field_position;
    u8 framebuffer_blue_mask_size;
} multiboot_info_t;

typedef struct __attribute__((packed)) {
    u32 size;
    u64 addr;
    u64 len;
#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5
    u32 type;
} multiboot_mmap_entry_t;

static inline char *mb_get_bootloader(multiboot_info_t *mbi) {
    if (mbi->flags & (1u << 9))
        return (char *)mbi->boot_loader_name;
    return (void *)0;
}
static inline u32 mb_lower_mem(multiboot_info_t *mbi) {
    if (mbi->flags & 1)
        return mbi->mem_lower;
    return 0;
}
static inline u32 mb_upper_mem(multiboot_info_t *mbi) {
    if (mbi->flags & 1)
        return mbi->mem_upper;
    return 0;
}

u64 mb_get_mem(multiboot_info_t *mbi);

#endif // KOR_BOOT_MULTIBOOT_H
