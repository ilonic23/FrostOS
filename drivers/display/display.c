#include "display.h"
#include "../../kernel/debug.h"
#include "../../libc/mem.h"
#include "../../multiboot/multiboot.h"
#include "vga.h"

static framebuffer_info fbi;
static vga_info_t *vi;
static uint8_t mode; // 0 = vga, 1 = framebuffer
static uint32_t cursor_x = 0, cursor_y = 0;
static display_font font;
static display_color col_fg = DISPLAY_COLOR(255, 255, 255),
                     col_bg = DISPLAY_COLOR(0, 0, 0);
static uint32_t scale = 1;

#define SPACING 4

void display_init_fb(multiboot_info_t *mbi) {
    mode = 1;
    fbi = framebuffer_init(mbi);
}
void display_init_vga(vga_info_t *info) {
    mode = 0;
    vi = info;
}

void display_put_pixel(uint32_t x, uint32_t y, display_color color) {
    if (mode) {
        uint32_t c = framebuffer_pack_color(&fbi, color.r, color.g, color.b);
        framebuffer_put_pixel(&fbi, x, y, c);
    } else
        vga_put_pixel(vi, x, y, color.r);
}

void display_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                       display_color color) {
    if (mode) {
        uint32_t c = framebuffer_pack_color(&fbi, color.r, color.g, color.b);
        framebuffer_fill_rect(&fbi, x, y, width, height, c);
    } else
        vga_fill_rect(vi, x, y, width, height, color.r);
}

void display_clear_screen() {
    if (mode) {
        uint32_t c = framebuffer_pack_color(&fbi, col_bg.r, col_bg.g, col_bg.b);
        framebuffer_clear_screen(&fbi, c);
    } else
        vga_clear_screen(vi, col_bg.r);
    cursor_x = 0;
    cursor_y = 0;
}

void display_put_char(uint32_t x, uint32_t y, display_font *font,
                      size_t char_index) {
    if (mode) {
        uint32_t fg = framebuffer_pack_color(&fbi, 255, 255, 255);
        uint32_t bg = framebuffer_pack_color(&fbi, 0, 0, 0);
        framebuffer_put_char(&fbi, font->glyphs, char_index, font->glyph_width,
                             font->glyph_height, x, y, fg, bg);
    } else
        vga_put_char(vi, font->glyphs, char_index, font->glyph_width,
                     font->glyph_height, x, y, WHITE_FG, BLACK_BG);
}

void display_put_char_ex(uint32_t x, uint32_t y, display_font *font,
                         size_t char_index, display_color color_fg,
                         display_color color_bg) {
    if (mode) {
        uint32_t fg =
            framebuffer_pack_color(&fbi, color_fg.r, color_fg.g, color_fg.b);
        uint32_t bg =
            framebuffer_pack_color(&fbi, color_bg.r, color_bg.g, color_bg.b);
        framebuffer_put_char(&fbi, font->glyphs, char_index, font->glyph_width,
                             font->glyph_height, x, y, fg, bg);
    } else
        vga_put_char(vi, font->glyphs, char_index, font->glyph_width,
                     font->glyph_height, x, y, color_fg.r, color_bg.r);
}

display_font display_get_font() { return font; }

void display_set_font(display_font *fnt) { font = *fnt; }

void display_set_cursor_pos(uint32_t x, uint32_t y) {
    cursor_x = x;
    cursor_y = y;
}

uint32_t display_get_cursor_x() { return cursor_x; }

uint32_t display_get_cursor_y() { return cursor_y; }

display_color display_get_foreground() { return col_fg; }

display_color display_get_background() { return col_bg; }

void display_set_foreground(display_color color) { col_fg = color; }

void display_set_background(display_color color) { col_bg = color; }

uint32_t display_get_width() { return (mode) ? fbi.width : vi->width; }

uint32_t display_get_height() { return (mode) ? fbi.height : vi->height; }

void display_print_char(char c, int32_t x, int32_t y) {
    if (x >= 0 && y >= 0) {
        cursor_x = x;
        cursor_y = y;
    }

    if (c == '\n') {
        cursor_y += font.glyph_height + SPACING;
        if (!mode) {
            if (cursor_y >= vi->height) {
                vga_scroll(vi, font.glyph_height + SPACING, col_bg.r);
                cursor_y -= 3 * (font.glyph_height + SPACING);
            }
        }
        cursor_x = 0;
    } else if (c == 0x08 || c == '\b') {
        if (cursor_x >= font.glyph_width) {
            cursor_x -= font.glyph_width;
            display_fill_rect(cursor_x, cursor_y, font.glyph_width,
                              font.glyph_height, col_bg);
        }
    } else {
        display_put_char_ex(cursor_x, cursor_y, &font, (uint8_t)c, col_fg,
                            col_bg);
        cursor_x += (vi && vi->mode == 0x3) ? 1 : font.glyph_width;
    }

    // Scroll or go to a new line
    if (cursor_x >= ((mode) ? fbi.width : vi->width)) {
        cursor_x = 0;
        cursor_y += font.glyph_height + SPACING;
    }
    if (cursor_y >= ((mode) ? fbi.height : vi->height)) {
        cursor_y = 0;
        cursor_x = 0;
        display_clear_screen();
    }
}

void display_print_char_ez(char c) { display_print_char(c, -1, -1); }

void display_print_str(const char *str) {
    for (size_t i = 0; str[i]; ++i)
        display_print_char(str[i], -1, -1);
}

void display_print_str_ex(const char *str, uint32_t x, uint32_t y,
                          display_color color_fg, display_color color_bg) {
    display_color orig_fg = col_fg, orig_bg = col_bg;
    col_fg = color_fg;
    col_bg = color_bg;
    cursor_x = x;
    cursor_y = y;
    for (size_t i = 0; str[i]; ++i)
        display_print_char(str[i], -1, -1);
    col_fg = orig_fg;
    col_bg = orig_bg;
}
