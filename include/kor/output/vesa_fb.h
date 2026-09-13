#ifndef KOR_OUTPUT_VESA_FB_H
#define KOR_OUTPUT_VESA_FB_H

#include <kor/boot/multiboot.h>
#include <kor/types.h>

typedef struct {
    u64 addr;
    u32 pitch;
    u32 width;
    u32 height;
    u8 bpp;
    u8 type;
    // color_info
    u8 red_field_position;
    u8 red_mask_size;
    u8 green_field_position;
    u8 green_mask_size;
    u8 blue_field_position;
    u8 blue_mask_size;
} framebuffer_info;

int framebuffer_supported(multiboot_info_t *mbi);
framebuffer_info framebuffer_init(multiboot_info_t *mbi);

u32 framebuffer_pack_color(framebuffer_info *info, u8 r, u8 g, u8 b);
void framebuffer_put_pixel(framebuffer_info *info, u32 x, u32 y, u32 color);
void framebuffer_fill_rect(framebuffer_info *info, u32 x, u32 y, u32 width,
                           u32 height, u32 color);
void framebuffer_clear_screen(framebuffer_info *info, u32 color);
void framebuffer_put_char(framebuffer_info *info, u8 *font, size char_index,
                          u32 char_width, u32 char_height, u32 x, u32 y,
                          u32 color_fg, u32 color_bg);

#endif
