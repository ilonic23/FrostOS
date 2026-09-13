#include <kor/output/vesa_fb.h>
#include <kor/types.h>

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

u32 framebuffer_pack_color(framebuffer_info *info, u8 r, u8 g, u8 b) {
    return ((u32)r >> (8 - info->red_mask_size) << info->red_field_position) |
           ((u32)g >> (8 - info->green_mask_size)
                          << info->green_field_position) |
           ((u32)b >> (8 - info->blue_mask_size) << info->blue_field_position);
}

void framebuffer_put_pixel(framebuffer_info *info, u32 x, u32 y, u32 color) {
    if (x >= info->width || y >= info->height)
        return;
    u8 *fb = (u8 *)info->addr;
    u32 *pixel = (u32 *)(fb + y * info->pitch + x * (info->bpp / 8));
    switch (info->bpp) {
    case 32:
        *(u32 *)pixel = color;
        break;
    case 24:
        pixel[0] = color & 0xFF;
        pixel[1] = (color >> 8) & 0xFF;
        pixel[2] = (color >> 16) & 0xFF;
        break;
    case 16:
        *(u16 *)pixel = (u16)color;
        break;
    }
}

void framebuffer_fill_rect(framebuffer_info *info, u32 x, u32 y, u32 width,
                           u32 height, u32 color) {
    for (u32 y_new = y; y_new < height + y; ++y_new)
        for (u32 x_new = x; x_new < width + x; ++x_new)
            framebuffer_put_pixel(info, x_new, y_new, color);
}

void framebuffer_clear_screen(framebuffer_info *info, u32 color) {
    // If no row padding
    if (info->pitch == info->width * 4) {
        u32 *fb = (u32 *)info->addr;
        u32 len = info->width * info->height;
        for (u32 i = 0; i < len; i++)
            fb[i] = color;
    } else {
        framebuffer_fill_rect(info, 0, 0, info->width, info->height, color);
    }
}

void framebuffer_put_char(framebuffer_info *info, u8 *font, size char_index,
                          u32 char_width, u32 char_height, u32 x, u32 y,
                          u32 color_fg, u32 color_bg) {
    u8(*fnt)[char_height] = (u8(*)[char_height])font;
    for (u32 row = 0; row < char_height; row++) {
        unsigned char byte = fnt[char_index][row];
        for (u32 col = 0; col < char_width; col++) {
            int bit = (byte >> ((char_width - 1) - col)) & 1;
            int px = x + col;
            int py = y + row;
            framebuffer_put_pixel(info, px, py, bit ? color_fg : color_bg);
        }
    }
}
