#ifndef KOR_OUTPUT_VGA_H
#define KOR_OUTPUT_VGA_H

#define VGA_VID_ADDR 0xB8000

#define VGA_MAX_ROWS 80
#define VGA_MAX_COLS 25

#define VGA_REG_SCREEN_CTRL 0x3D4
#define VGA_REG_SCREEN_DATA 0x3D5

#include <kor/types.h>

typedef struct {
    u32 width;
    u32 height;
    u8 *base_addr;
    u8 mode;
} vga_info_t;

vga_info_t vga_init(u8 mode);
void vga_copy_font(volatile u8 *dest);
void vga_set_font(u8 *font);
void vga_set_mode_13h(vga_info_t *info);
void vga_set_mode_3h(vga_info_t *info);
void vga_set_grayscale_cols();
void vga_set_xterm_cols();
void vga_set_text_colors();
void vga_put_pixel(vga_info_t *info, u32 x, u32 y, u8 color);
void vga_fill_rect(vga_info_t *info, u32 x, u32 y, u32 width, u32 height,
                   u8 color);
void vga_scroll(vga_info_t *info, u32 y_pixels, u8 bg);
void vga_put_char(vga_info_t *info, u8 *font, size char_index, u32 char_width,
                  u32 char_height, u32 x, u32 y, u32 color_fg, u32 color_bg);
void vga_clear_screen(vga_info_t *info, u8 color);
void vga_print(char *str);
void vga_print_at(char *str, int col, int row);
void vga_print_at_attr(char *str, int col, int row, char attr);
void vga_print_char(char str);
void vga_print_attr(char *str, char attr);
void vga_print_backspace();
int vga_get_cursor_offset();
void vga_set_cursor_offset(int offset);
int vga_get_offset(int col, int row);

enum {
    BLACK_FG = 0x00,
    BLUE_FG = 0x01,
    GREEN_FG = 0x02,
    CYAN_FG = 0x03,
    RED_FG = 0x04,
    MAGENTA_FG = 0x05,
    BROWN_FG,
    LIGHTGRAY_FG,
    DARKGRAY_FG,
    LIGHTBLUE_FG,
    LIGHTGREEN_FG,
    LIGHTCYAN_FG,
    LIGHTRED_FG,
    LIGHTMAGENTA_FG,
    YELLOW_FG,
    WHITE_FG
};

enum {
    BLACK_BG = 0x00,
    BLUE_BG = 0x10,
    GREEN_BG = 0x20,
    CYAN_BG = 0x30,
    RED_BG = 0x40,
    MAGENTA_BG = 0x50,
    BROWN_BG = 0x60,
    LIGHTGRAY_BG = 0x70,
    DARKGRAY_BG = 0x80,
    LIGHTBLUE_BG = 0x90,
    LIGHTGREEN_BG = 0xA0,
    LIGHTCYAN_BG = 0xB0,
    LIGHTRED_BG = 0xC0,
    LIGHTMAGENTA_BG = 0xD0,
    YELLOW_BG = 0xB0,
    WHITE_BG = 0xB0
};

enum {
    BoxCrnLeftUp = 0xC9,
    BoxCrnRightUp = 0xBB,
    BoxCrnLeftDown = 0xC8,
    BoxCrnRightDown = 0xBC,
    BoxLineVert = 0xBA,
    BoxLineHorz = 0xCD,
    BoxVer2HorzLeft = 0xB9,
    BoxVer2HorzRight = 0xCC,
    BoxHorz2VerUp = 0xCA,
    BoxHorz2VerDown = 0xCB,
    BoxCross = 0xCE
};

#endif // KOR_OUTPUT_VGA_H
