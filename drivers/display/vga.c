#include "vga.h"
#include "../../cpu/ports.h"
#include "../../libc/mem.h"
#include <stdint.h>

#define ATTRIB_CTL_W 0x3C0
#define ATTRIB_CTL_R 0x3C1
#define MISCOUT_W 0x3C2
#define MISCOUT_R 0x3CC
#define SEQ_RW 0x3C4
#define GRAPH_CTL_RW 0x3CE
#define CRT_CTL_RW 0x3D4
#define DAC_MASK_RW 0x3C6
#define DAC_IDX_READ_W 0x3C7
#define DAC_IDX_WRITE_W 0x3C8
#define DAC_COL_RW 0x3C9

void vga_attrib_ctl_w(uint8_t index, uint8_t value) {
    // reset index/value state
    (volatile void)port_byte_in(0x3DA);
    port_byte_out(ATTRIB_CTL_W, index);
    port_byte_out(ATTRIB_CTL_W, value);
}
uint8_t vga_attrib_ctl_r(uint8_t index) {
    (volatile void)port_byte_in(0x3DA);
    port_byte_out(ATTRIB_CTL_W, index);
    return port_byte_in(ATTRIB_CTL_R);
}

void vga_miscout_w(uint8_t value) { port_byte_out(MISCOUT_W, value); }
uint8_t vga_miscout_r() { return port_byte_in(MISCOUT_R); }

// remap CRT_CTL_RW from 0x3B4 to 0x3D4
void vga_remap_crt_ctl() {
    uint8_t miscout = vga_miscout_r();
    miscout |= (1u << 0);
    vga_miscout_w(miscout);
}

void vga_seq_w(uint8_t index, uint8_t value) {
    port_word_out(SEQ_RW, ((uint16_t)value) << 8 | (uint16_t)index);
}
uint8_t vga_seq_r(uint8_t index) {
    port_byte_out(SEQ_RW, index);
    return port_byte_in(SEQ_RW + 1);
}

void vga_graph_ctl_w(uint8_t index, uint8_t value) {
    port_word_out(GRAPH_CTL_RW, ((uint16_t)value) << 8 | (uint16_t)index);
}
uint8_t vga_graph_ctl_r(uint8_t index) {
    port_byte_out(GRAPH_CTL_RW, index);
    return port_byte_in(GRAPH_CTL_RW + 1);
}

void vga_crt_ctl_w(uint8_t index, uint8_t value) {
    port_word_out(CRT_CTL_RW, ((uint16_t)value) << 8 | (uint16_t)index);
}
uint8_t vga_crt_ctl_r(uint8_t index) {
    port_byte_out(CRT_CTL_RW, index);
    return port_byte_in(CRT_CTL_RW + 1);
}

void vga_dac_mask_w(uint8_t value) { port_byte_out(DAC_MASK_RW, value); }
uint8_t vga_dac_mask_r() { return port_byte_in(DAC_MASK_RW); }

void vga_dac_idx_read_w(uint8_t index) { port_byte_out(DAC_IDX_READ_W, index); }
void vga_dac_idx_write_w(uint8_t index) {
    port_byte_out(DAC_IDX_WRITE_W, index);
}
void vga_dac_col_w(uint8_t value) { port_byte_out(DAC_COL_RW, value); }
uint8_t vga_dac_col_r() { return port_byte_in(DAC_COL_RW); }

int get_offset_row(int offset);
int get_offset_col(int offset);
static int print_char(char c, int col, int row, char attr);

void vga_print_at(char *str, int col, int row) {
    vga_print_at_attr(str, col, row, BLACK_BG | WHITE_FG);
}

void vga_print_at_attr(char *str, int col, int row, char attr) {
    int offset;
    if (col >= 0 && row >= 0)
        offset = vga_get_offset(col, row);
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

void vga_print(char *str) { vga_print_at(str, -1, -1); }

void vga_print_char(char c) { print_char(c, -1, -1, BLACK_BG | WHITE_FG); }

void vga_print_attr(char *str, char attr) {
    vga_print_at_attr(str, -1, -1, attr);
}

void vga_print_backspace() {
    int offset = vga_get_cursor_offset() - 2;
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

int vga_get_offset(int col, int row) { return 2 * (row * VGA_MAX_COLS + col); }

static int print_char(char c, int col, int row, char attr) {
    uint8_t *screen = (uint8_t *)VGA_VID_ADDR;
    if (!attr)
        attr = BLACK_BG | WHITE_FG;

    if (col >= VGA_MAX_COLS || row >= VGA_MAX_ROWS) {
        screen[2 * (VGA_MAX_COLS) * (VGA_MAX_ROWS)-2] = 'E';
        screen[2 * (VGA_MAX_COLS) * (VGA_MAX_ROWS)-1] = BLACK_BG | RED_FG;
        return vga_get_offset(col, row);
    }

    int offset;
    if (col >= 0 && row >= 0)
        offset = vga_get_offset(col, row);
    else
        offset = vga_get_cursor_offset();

    if (c == '\n') {
        row = get_offset_row(offset);
        offset = vga_get_offset(0, row + 1);
    } else if (c == 0x08 || c == '\b') {
        screen[offset - 2] = ' ';
        screen[offset - 1] = attr;
        offset -= 2;
    } else {
        screen[offset] = c;
        screen[offset + 1] = attr;
        offset += 2;
    }

    // Check if the offset is over screen size and roll
    if (offset >= VGA_MAX_ROWS * VGA_MAX_COLS * 2) {
        for (int i = 1; i < VGA_MAX_ROWS; ++i)
            memcpy((uint8_t *)(vga_get_offset(0, i - 1) + VGA_VID_ADDR),
                   (uint8_t *)(vga_get_offset(0, i) + VGA_VID_ADDR),
                   VGA_MAX_COLS * 2);

        char *last_line = (char *)(vga_get_offset(0, VGA_MAX_ROWS - 1) +
                                   (uint8_t *)VGA_VID_ADDR);

        for (int i = 0; i < VGA_MAX_COLS * 2; ++i)
            last_line[i] = 0;
        offset -= 2 * VGA_MAX_COLS;
    }

    vga_set_cursor_offset(offset);
    return offset;
}

void vga_clear_screen() {
    uint8_t *screen = (uint8_t *)VGA_VID_ADDR;

    for (int i = 0; i < VGA_MAX_COLS * VGA_MAX_ROWS; ++i) {
        screen[i * 2] = ' ';
        screen[i * 2 + 1] = BLACK_BG | WHITE_FG;
    }
    vga_set_cursor_offset(vga_get_offset(0, 0));
}

int get_offset_row(int offset) { return offset / (2 * VGA_MAX_COLS); }
int get_offset_col(int offset) {
    return (offset - (get_offset_row(offset) * 2 * VGA_MAX_COLS)) / 2;
}
