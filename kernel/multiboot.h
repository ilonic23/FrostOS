#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t flags;

    // flags bit 0
    uint32_t mem_lower;
    uint32_t mem_upper;

    // flags bit 1
    uint32_t boot_device;

    // flags bit 2
    uint32_t cmdline;

    // flags bit 3
    uint32_t mods_count;
    uint32_t mods_addr;

    // flags bits 4/5 (symbols)
    uint32_t syms[4];

    // flags bit 6 (memory map)
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // flags bit 7
    uint32_t drives_length;
    uint32_t drives_addr;
    
    // flags bit 8
    uint32_t config_table;

    // flags bit 9
    uint32_t boot_loader_name;

    // flags bit 10
    uint32_t apm_table;

    // flags bit 11 -- VBE info
    uint32_t vbe_control_info;   // physical addr of VbeInfoBlock
    uint32_t vbe_mode_info;      // physical addr of ModeInfoBlock
    uint16_t vbe_mode;           // current VBE mode number
    uint16_t vbe_interface_seg;  // PMI: real-mode segment
    uint16_t vbe_interface_off;  // PMI: offset within segment
    uint16_t vbe_interface_len;  // PMI: length in bytes
                                 
    // flags bit 12 -- Framebuffer
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    // color_info
    uint8_t  framebuffer_red_field_position;
    uint8_t  framebuffer_red_mask_size;
    uint8_t  framebuffer_green_field_position;
    uint8_t  framebuffer_green_mask_size;
    uint8_t  framebuffer_blue_field_position;
    uint8_t  framebuffer_blue_mask_size;
} multiboot_info_t;

char *mb_get_bootloader(multiboot_info_t *mbi);
uint64_t mb_get_lower_mem(multiboot_info_t *mbi);
uint64_t mb_get_upper_mem(multiboot_info_t *mbi);

#endif
