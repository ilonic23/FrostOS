#include "framebuffer.h"
#include <stdint.h>

int framebuffer_supported(multiboot_info_t *mbi) {
    return (mbi->flags & (1u << 12)) != 0;
}

framebuffer_info framebuffer_init(multiboot_info_t *mbi) {
    return (framebuffer_info){
        .addr = mbi->framebuffer_addr,
        .pitch = mbi->framebuffer_pitch,
        .width = mbi->framebuffer_width,
        .height = mbi->framebuffer_height,
        .bpp = mbi->framebuffer_bpp,
        .type = mbi->framebuffer_type,
        .red_field_position = mbi->framebuffer_red_field_position,
        .red_mask_size = mbi->framebuffer_red_mask_size,
        .green_field_position = mbi->framebuffer_green_field_position,
        .green_mask_size = mbi->framebuffer_green_mask_size,
        .blue_field_position = mbi->framebuffer_blue_field_position,
        .blue_mask_size = mbi->framebuffer_blue_mask_size,
    };
}

uint32_t framebuffer_pack_color(framebuffer_info *info, uint8_t r, uint8_t g,
                                uint8_t b) {
    return ((uint32_t)r >> (8 - info->red_mask_size)
                               << info->red_field_position) |
           ((uint32_t)g >> (8 - info->green_mask_size)
                               << info->green_field_position) |
           ((uint32_t)b >> (8 - info->blue_mask_size)
                               << info->blue_field_position);
}

void framebuffer_put_pixel(framebuffer_info *info, uint32_t x, uint32_t y,
                           uint32_t color) {
    if (x >= info->width || y >= info->height)
        return;
    uint8_t *fb = (uint8_t *)(uintptr_t)info->addr;
    uint32_t *pixel = (uint32_t *)(fb + y * info->pitch + x * (info->bpp / 8));
    switch (info->bpp) {
    case 32:
        *(uint32_t *)pixel = color;
        break;
    case 24:
        pixel[0] = color & 0xFF;
        pixel[1] = (color >> 8) & 0xFF;
        pixel[2] = (color >> 16) & 0xFF;
        break;
    case 16:
        *(uint16_t *)pixel = (uint16_t)color;
        break;
    }
}

void framebuffer_fill_rect(framebuffer_info *info, uint32_t x, uint32_t y,
                           uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t y_new = y; y_new < height + y; ++y_new)
        for (uint32_t x_new = x; x_new < width + x; ++x_new)
            framebuffer_put_pixel(info, x_new, y_new, color);
}

void framebuffer_clear_screen(framebuffer_info *info, uint32_t color) {
    // If no row padding
    if (info->pitch == info->width * 4) {
        uint32_t *fb = (uint32_t *)(uintptr_t)info->addr;
        uint32_t len = info->width * info->height;
        for (uint32_t i = 0; i < len; i++)
            fb[i] = color;
    } else {
        framebuffer_fill_rect(info, 0, 0, info->width, info->height, color);
    }
}

void framebuffer_put_char(framebuffer_info *info, uint8_t *font,
                          size_t char_index, uint32_t char_width,
                          uint32_t char_height, uint32_t x, uint32_t y,
                          uint32_t color_fg, uint32_t color_bg) {
    uint8_t (*fnt)[char_height] = (uint8_t (*)[char_height])font;
    for (uint32_t row = 0; row < char_height; row++) {
        unsigned char byte = fnt[char_index][row];
        for (uint32_t col = 0; col < char_width; col++) {
            int bit = (byte >> ((char_width - 1) - col)) & 1;
            int px = x + col;
            int py = y + row;
            framebuffer_put_pixel(info, px, py, bit ? color_fg : color_bg);
        }
    }
}
