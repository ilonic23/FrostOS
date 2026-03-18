#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include "../kernel/multiboot.h"

/* TODO:
 * 1. Put pixel - Done
 * 2. Fill rect - Done
 * 3. Gather info
 * 4. Draw something
 * 5. Put char
*/

typedef struct {
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t  bpp;
    uint8_t  type;
    // color_info
    uint8_t  red_field_position;
    uint8_t  red_mask_size;
    uint8_t  green_field_position;
    uint8_t  green_mask_size;
    uint8_t  blue_field_position;
    uint8_t  blue_mask_size;
} framebuffer_info;

int framebuffer_supported(multiboot_info_t *mbi);
framebuffer_info framebuffer_init(multiboot_info_t *mbi);

uint32_t framebuffer_pack_color(framebuffer_info *info, uint8_t r, uint8_t g, uint8_t b);
void framebuffer_put_pixel(framebuffer_info *info, uint32_t x, uint32_t y, uint32_t color);
void framebuffer_fill_rect(framebuffer_info *info, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, uint32_t color);
void framebuffer_clear_screen(framebuffer_info *info, uint32_t color);

#endif
