#include "vga.h"
#include <stdint.h>
#include "../libc/mem.h"
#include "../cpu/ports.h"

int get_offset_row(int offset);
int get_offset_col(int offset);
static int print_char(char c, int col, int row, char attr);

void vga_print_at(char *str, int col, int row) {
    vga_print_at_attr(str, col, row, BLACK_BG | WHITE_FG);
}

void vga_print_at_attr(char *str, int col, int row, char attr) {
    int offset;
    if (col >= 0 && row >= 0)
        offset = vga_col_row_to_offset(col, row);
    else {
        offset = vga_get_cursor_offset();
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }

    for (int i = 0; str[i];) {
        offset = print_char(str[i++], col, row, attr);
        /* Compute row/col for next iteration */
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
}

void vga_print(char *str) {
    vga_print_at(str, -1, -1);
}

void vga_print_char(char c) {
    print_char(c, -1, -1, BLACK_BG | WHITE_FG);
}

void vga_print_attr(char *str, char attr) {
    vga_print_at_attr(str, -1, -1, attr);
}

void vga_print_backspace() {
    int offset = vga_get_cursor_offset()-2;
    int row = get_offset_row(offset);
    int col = get_offset_col(offset);
    print_char(0x08, col, row, BLACK_BG | WHITE_FG);
}

int vga_get_cursor_offset() {
    port_byte_out(VGA_REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(VGA_REG_SCREEN_DATA) << 8;
    port_byte_out(VGA_REG_SCREEN_CTRL, 15);
    offset += port_byte_in(VGA_REG_SCREEN_DATA);
    return offset * 2;
}

void vga_set_cursor_offset(int offset) {
    // Similar to get_cursor_offset, but instead of reading we write data
    offset /= 2;
    port_byte_out(VGA_REG_SCREEN_CTRL, 14);
    port_byte_out(VGA_REG_SCREEN_DATA, (uint8_t)(offset >> 8));
    port_byte_out(VGA_REG_SCREEN_CTRL, 15);
    port_byte_out(VGA_REG_SCREEN_DATA, (uint8_t)(offset & 0xff));
}

int vga_col_row_to_offset(int col, int row) { return 2 * (row * VGA_MAX_COLS + col); }

static int print_char(char c, int col, int row, char attr) {
    uint8_t *screen = (uint8_t *)VGA_VID_ADDR;
    if (!attr) attr = BLACK_BG | WHITE_FG;

    if (col >= VGA_MAX_ROWS || row >= VGA_MAX_ROWS) {
        screen[2*VGA_MAX_COLS*VGA_MAX_ROWS-2] = 'E';
        screen[2*VGA_MAX_COLS*VGA_MAX_ROWS-1] = BLACK_BG | RED_FG;
        return vga_col_row_to_offset(col, row);
    }

    int offset;
    if (col >= 0 && row >= 0) offset = vga_col_row_to_offset(col, row);
    else offset = vga_get_cursor_offset();

    if (c == '\n') {
        row = get_offset_row(offset);
        offset = vga_col_row_to_offset(col, row);
    } else if (c == 0x08 || c == '\b') {
        screen[offset-2] = ' ';
        screen[offset-1] = attr;
        offset-=2;
    } else {
        screen[offset] = c;
        screen[offset+1] = attr;
        offset += 2;
    }

    // Check if the offset is over screen size and roll
    if (offset >= VGA_MAX_ROWS * VGA_MAX_COLS * 2) {
        for (int i = 1; i < VGA_MAX_ROWS; ++i)
            memcpy((uint8_t*)(vga_col_row_to_offset(0, i) + VGA_VID_ADDR),
                        (uint8_t*)(vga_col_row_to_offset(0, i-1) + VGA_VID_ADDR),
                        VGA_MAX_COLS * 2);
        char *last_line = (char *)(vga_col_row_to_offset(0, VGA_MAX_ROWS-1)
                + (uint8_t *)VGA_VID_ADDR);

        for (int i = 0; i < VGA_MAX_COLS *2; ++i) last_line[i] = 0;
        offset -= 2*VGA_MAX_COLS;
    }

    vga_set_cursor_offset(offset);
    return offset;
}

void vga_clear_screen() {
    uint8_t *screen = (uint8_t *)VGA_VID_ADDR;

    for (int i = 0; i < VGA_MAX_COLS * VGA_MAX_ROWS; ++i) {
        screen[i*2] = ' ';
        screen[i*2+1] = BLACK_BG | WHITE_FG;
    }
    vga_set_cursor_offset(vga_col_row_to_offset(0, 0));
}

int get_offset_row(int offset) { return offset / (2 * VGA_MAX_COLS); }
int get_offset_col(int offset) { return (offset - (get_offset_row(offset)*2*VGA_MAX_COLS))/2; }
