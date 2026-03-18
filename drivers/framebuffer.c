#include "framebuffer.h"
#include <stdint.h>
#include "../libc/mem.h"

int framebuffer_supported(multiboot_info_t *mbi) {
    return (mbi->flags & (1u << 12)) != 0;
}

framebuffer_info framebuffer_init(multiboot_info_t *mbi) {
    return (framebuffer_info) {
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

uint32_t framebuffer_pack_color(framebuffer_info *info, uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << info->red_field_position)
     | ((uint32_t)g << info->green_field_position)
     | ((uint32_t)b << info->blue_field_position);
}

void framebuffer_put_pixel(framebuffer_info *info, uint32_t x, uint32_t y, uint32_t color) {
    uint8_t *fb = (uint8_t*)(uintptr_t)info->addr;
    uint32_t *pixel = (uint32_t*)(fb + y * info->pitch + x * 4);
    *pixel = color;
}

void framebuffer_fill_rect(framebuffer_info *info, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, uint32_t color) {
    for (int x_new = x; x_new + x < width; ++x_new)
        for (int y_new = y; y_new + y < height; ++y_new)
            framebuffer_put_pixel(info, x_new, y_new, color);
}

void framebuffer_clear_screen(framebuffer_info *info, uint32_t color) {
    // If no row padding
    if (info->pitch == info->width * 4) {
        uint32_t *fb  = (uint32_t*)info->addr;
        uint32_t  len = info->width * info->height;
        for (uint32_t i = 0; i < len; i++)
            fb[i] = color;
    } else {
        framebuffer_fill_rect(info, 0, 0, info->width, info->height, color);
    }
}
