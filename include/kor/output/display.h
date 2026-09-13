#ifndef KOR_OUTPUT_DISPLAY_H
#define KOR_OUTPUT_DISPLAY_H

#include <kor/boot/multiboot.h>
#include <kor/output/vesa_fb.h>
#include <kor/output/vga.h>
#include <kor/types.h>

typedef struct {
    u8 r;
    u8 g;
    u8 b;
} display_color;

#define DISPLAY_COLOR(red, green, blue)                                        \
    (display_color) { .r = (red), .g = (green), .b = (blue) }
#define VGA_COLOR(c) DISPLAY_COLOR(c, c, c)

typedef struct {
    u8 *glyphs;
    size glyph_count;
    u32 glyph_width;
    u32 glyph_height;
    u8 map[255];
} display_font;

void display_init_fb(multiboot_info_t *mbi);
void display_init_vga(vga_info_t *info);
void display_put_pixel(u32 x, u32 y, display_color color);
void display_fill_rect(u32 x, u32 y, u32 width, u32 height,
                       display_color color);
void display_clear_screen();
void display_put_char(u32 x, u32 y, display_font *font, size char_index);
void display_put_char_ex(u32 x, u32 y, display_font *font, size char_index,
                         display_color color_fg, display_color color_bg);
void display_set_font(display_font *fnt);
display_font display_get_font();
display_color display_get_foreground();
display_color display_get_background();
void display_set_cursor_pos(u32 x, u32 y);
u32 display_get_cursor_x();
u32 display_get_cursor_y();
u32 display_get_width();
u32 display_get_height();
void display_set_foreground(display_color color);
void display_set_background(display_color color);
void display_print_char(char c, i32 x, i32 y);
void display_print_char_ez(char c);
void display_print_str(const char *str);
void display_print_str_ex(const char *str, u32 x, u32 y, display_color color_fg,
                          display_color color_bg);

#endif // KOR_OUTPUT_DISPLAY_H
