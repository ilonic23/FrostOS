#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stddef.h>

#include "framebuffer.h"
//#include "../../drivers/vga.h"
#include "../../multiboot/multiboot.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} display_color;

#define DISPLAY_COLOR(red,green,blue) (display_color){ .r = (blue), .g = (red), .b = (green) } // There's a weird bug that text colors appear right only so.

typedef struct {
    uint8_t  *glyphs;
    size_t    glyph_count;
    uint32_t  glyph_width;
    uint32_t  glyph_height;
    uint8_t   map[255];
} display_font;

void display_init(multiboot_info_t *mbi);
void display_put_pixel(uint32_t x, uint32_t y, display_color color);
void display_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        display_color color);
void display_clear_screen(display_color color);
void display_put_char(uint32_t x, uint32_t y, display_font *font, size_t char_index);
void display_put_char_ex(uint32_t x, uint32_t y, display_font *font, size_t char_index,
        display_color color_fg, display_color color_bg);
void display_set_font(display_font *fnt);
display_font display_get_font();
display_color display_get_foreground();
display_color display_get_background();
void display_set_cursor_pos(uint32_t x, uint32_t y);
uint32_t display_get_cursor_x();
uint32_t display_get_cursor_y();
uint32_t display_get_width();
uint32_t display_get_height();
void display_set_foreground(display_color color);
void display_set_background(display_color color);
void display_print_char(char c, int32_t x, int32_t y);
void display_print_char_ez (char c);
void display_print_str(const char *str);
void display_print_str_ex(const char *str, uint32_t x, uint32_t y,
        display_color color_fg, display_color color_bg);

#endif
