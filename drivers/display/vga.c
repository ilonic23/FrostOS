#include "../../libc/mem.h"
#include <kor/ll/port.h>
#include <kor/output/vga.h>
#include <kor/types.h>

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

vga_info_t vga_init(u8 mode) {
    vga_info_t result = {
        .mode = mode,
    };
    if (mode == 0x3) {
        result.width = 80;
        result.height = 25;
        result.base_addr = (u8 *)0xB8000;
    } else if (mode == 0x13) {
        result.width = 320;
        result.height = 200;
        result.base_addr = (u8 *)0xA0000;
    }
    return result;
}

void vga_attrib_ctl_w(u8 index, u8 value) {
    // reset index/value state
    (volatile void)inb(0x3DA);
    outb(ATTRIB_CTL_W, index);
    outb(ATTRIB_CTL_W, value);
}
u8 vga_attrib_ctl_r(u8 index) {
    (volatile void)inb(0x3DA);
    outb(ATTRIB_CTL_W, index);
    return inb(ATTRIB_CTL_R);
}

void vga_miscout_w(u8 value) { outb(MISCOUT_W, value); }
u8 vga_miscout_r() { return inb(MISCOUT_R); }

// remap CRT_CTL_RW from 0x3B4 to 0x3D4
void vga_remap_crt_ctl() {
    u8 miscout = vga_miscout_r();
    miscout |= (1u << 0);
    vga_miscout_w(miscout);
}

void vga_seq_w(u8 index, u8 value) {
    outw(SEQ_RW, ((u16)value) << 8 | (u16)index);
}
u8 vga_seq_r(u8 index) {
    outb(SEQ_RW, index);
    return inb(SEQ_RW + 1);
}

void vga_graph_ctl_w(u8 index, u8 value) {
    outw(GRAPH_CTL_RW, ((u16)value) << 8 | (u16)index);
}
u8 vga_graph_ctl_r(u8 index) {
    outb(GRAPH_CTL_RW, index);
    return inb(GRAPH_CTL_RW + 1);
}

void vga_crt_ctl_w(u8 index, u8 value) {
    outw(CRT_CTL_RW, ((u16)value) << 8 | (u16)index);
}
u8 vga_crt_ctl_r(u8 index) {
    outb(CRT_CTL_RW, index);
    return inb(CRT_CTL_RW + 1);
}

void vga_dac_mask_w(u8 value) { outb(DAC_MASK_RW, value); }
u8 vga_dac_mask_r() { return inb(DAC_MASK_RW); }

void vga_dac_idx_read_w(u8 index) { outb(DAC_IDX_READ_W, index); }
void vga_dac_idx_write_w(u8 index) { outb(DAC_IDX_WRITE_W, index); }
void vga_dac_col_w(u8 value) { outb(DAC_COL_RW, value); }
u8 vga_dac_col_r() { return inb(DAC_COL_RW); }

// If in mode 3h, copies the 8x16 font
// Code rewritten from:
// https://wiki.osdev.org/VGA_Fonts#Get_from_VGA_RAM_directly
void vga_copy_font(volatile u8 *dest) {
    u8 seq2 = vga_seq_r(0x02), seq4 = vga_seq_r(0x04);
    u8 gc4 = vga_graph_ctl_r(0x04);
    u8 gc5 = vga_graph_ctl_r(0x05);
    u8 gc6 = vga_graph_ctl_r(0x06);

    vga_seq_w(0x02, 0x04);
    vga_seq_w(0x04, seq4 | 0x04);

    vga_graph_ctl_w(0x04, 0x02);
    vga_graph_ctl_w(0x05, gc5 & ~0x10);
    vga_graph_ctl_w(0x06, (gc6 & ~0x0C) | 0x04);

    u8 *vga_mem = (u8 *)0xA0000;
    for (int i = 0; i < 256; ++i)
        for (int row = 0; row < 16; ++row)
            dest[i * 16 + row] = vga_mem[i * 32 + row];

    vga_seq_w(0x02, seq2);
    vga_seq_w(0x04, seq4);
    vga_graph_ctl_w(0x04, gc4);
    vga_graph_ctl_w(0x05, gc5);
    vga_graph_ctl_w(0x06, gc6);
}

void vga_set_font(u8 *font) {
    vga_seq_w(0x02, 0x04); // map mask: plane 2 only
    vga_seq_w(
        0x04,
        0x06); // disable odd/even, enable extended memory (linear plane access)
    vga_graph_ctl_w(0x05, 0x00); // disable odd/even on read side
    vga_graph_ctl_w(0x06, 0x04); // map to 0xA0000, no odd/even
    memcpy((u8 *)0xA0000, font, 256 * 16);

    vga_seq_w(0x02, 0x0F);
    vga_seq_w(0x04, 0x02);
    vga_graph_ctl_w(0x05, 0x10);
    vga_graph_ctl_w(0x06, 0x0E);
}

void vga_set_mode_3h(vga_info_t *info) {
    vga_attrib_ctl_w(0x10, 0x0C);
    vga_attrib_ctl_w(0x11, 0x00);
    vga_attrib_ctl_w(0x12, 0x0F);
    vga_attrib_ctl_w(0x13, 0x08);
    vga_attrib_ctl_w(0x14, 0x00);
    vga_miscout_w(0x67);
    vga_seq_w(0x01, 0x01);
    vga_seq_w(0x02, 0x0F);
    vga_seq_w(0x03, 0x03);
    vga_seq_w(0x04, 0x02); // 0x2 / 0x7 ???
    vga_graph_ctl_w(0x05, 0x10);
    vga_graph_ctl_w(0x06, 0x0E);
    vga_remap_crt_ctl();
    vga_crt_ctl_w(0x00, 0x5F);
    vga_crt_ctl_w(0x01, 0x4F);
    vga_crt_ctl_w(0x02, 0x50);
    vga_crt_ctl_w(0x03, 0x82);
    vga_crt_ctl_w(0x04, 0x55);
    vga_crt_ctl_w(0x05, 0x81);
    vga_crt_ctl_w(0x06, 0xBF);
    vga_crt_ctl_w(0x07, 0x1F);
    vga_crt_ctl_w(0x08, 0x00);
    vga_crt_ctl_w(0x09, 0x4F);
    vga_crt_ctl_w(0x10, 0x9C);
    vga_crt_ctl_w(0x11, 0x8E);
    vga_crt_ctl_w(0x12, 0x8F);
    vga_crt_ctl_w(0x13, 0x28);
    vga_crt_ctl_w(0x14, 0x1F);
    vga_crt_ctl_w(0x15, 0x96);
    vga_crt_ctl_w(0x16, 0xB9);
    vga_crt_ctl_w(0x17, 0xA3);
    (volatile void)vga_attrib_ctl_r(0x20);
    info->width = 80;
    info->height = 25;
    info->base_addr = (u8 *)0xB8000;
    info->mode = 0x03;
}

void vga_set_mode_13h(vga_info_t *info) {
    vga_attrib_ctl_w(0x10, 0x41);
    vga_attrib_ctl_w(0x11, 0x00);
    vga_attrib_ctl_w(0x12, 0x0F);
    vga_attrib_ctl_w(0x13, 0x00);
    vga_attrib_ctl_w(0x14, 0x00);
    vga_miscout_w(0x63);
    vga_seq_w(0x01, 0x01);
    vga_seq_w(0x02, 0x0F);
    vga_seq_w(0x03, 0x00);
    vga_seq_w(0x04, 0x0E);
    vga_graph_ctl_w(0x05, 0x40);
    vga_graph_ctl_w(0x06, 0x05);
    vga_remap_crt_ctl();
    vga_crt_ctl_w(0x00, 0x5F);
    vga_crt_ctl_w(0x01, 0x4F);
    vga_crt_ctl_w(0x02, 0x50);
    vga_crt_ctl_w(0x03, 0x82);
    vga_crt_ctl_w(0x04, 0x54);
    vga_crt_ctl_w(0x05, 0x80);
    vga_crt_ctl_w(0x06, 0xBF);
    vga_crt_ctl_w(0x07, 0x1F);
    vga_crt_ctl_w(0x08, 0x00);
    vga_crt_ctl_w(0x09, 0x41);
    vga_crt_ctl_w(0x10, 0x9C);
    vga_crt_ctl_w(0x11, 0x8E);
    vga_crt_ctl_w(0x12, 0x8F);
    vga_crt_ctl_w(0x13, 0x28);
    vga_crt_ctl_w(0x14, 0x40);
    vga_crt_ctl_w(0x15, 0x96);
    vga_crt_ctl_w(0x16, 0xB9);
    vga_crt_ctl_w(0x17, 0xA3);
    (volatile void)vga_attrib_ctl_r(0x20);
    info->width = 320;
    info->height = 200;
    info->base_addr = (u8 *)0xA0000;
    info->mode = 0x13;
}

void vga_set_grayscale_cols() {
    vga_dac_idx_write_w(0);
    for (int i = 0; i < 256; ++i) {
        vga_dac_col_w(i >> 2);
        vga_dac_col_w(i >> 2);
        vga_dac_col_w(i >> 2);
    }
}

void vga_set_xterm_cols() {
    vga_dac_idx_write_w(0);
    int i;
    for (i = 0; i < 16; ++i) {
        u8 bit0 = (i & (1u << 0)) >> 0;
        u8 bit1 = (i & (1u << 1)) >> 1;
        u8 bit2 = (i & (1u << 2)) >> 2;
        if (i < 8) {
            vga_dac_col_w(bit2 ? 43 : 0);                 // 170/255*64 ~= 43;
            vga_dac_col_w(i == 6 ? 21 : (bit1 ? 43 : 0)); // 85/255*64 ~= 21;
            vga_dac_col_w(bit0 ? 43 : 0);
        } else {
            vga_dac_col_w(bit2 ? 63 : 21);
            vga_dac_col_w(bit1 ? 63 : 21);
            vga_dac_col_w(bit0 ? 63 : 21);
        }
    }
    for (; i < 232; ++i) {
        int t = i - 16;
        int r_idx = t / 36;
        int g_idx = (t % 36) / 6;
        int b_idx = t % 6;

        vga_dac_col_w((r_idx * 51) / 4); // idx(0-5) -> 0-255 -> 0-63
        vga_dac_col_w((g_idx * 51) / 4);
        vga_dac_col_w((b_idx * 51) / 4);
    }
    for (int j = 0; i < 256; ++i, j++) {
        vga_dac_col_w(j * 3);
        vga_dac_col_w(j * 3);
        vga_dac_col_w(j * 3);
    }
}

void vga_set_text_colors() {
    static const u8 palette[16][3] = {
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x2A}, {0x00, 0x2A, 0x00},
        {0x00, 0x2A, 0x2A}, {0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x2A},
        {0x2A, 0x15, 0x00}, {0x2A, 0x2A, 0x2A}, {0x15, 0x15, 0x15},
        {0x15, 0x15, 0x3F}, {0x15, 0x3F, 0x15}, {0x15, 0x3F, 0x3F},
        {0x3F, 0x15, 0x15}, {0x3F, 0x15, 0x3F}, {0x3F, 0x3F, 0x15},
        {0x3F, 0x3F, 0x3F},
    };
    vga_dac_idx_write_w(0);
    for (int i = 0; i < 16; ++i) {
        vga_dac_col_w(palette[i][0]);
        vga_dac_col_w(palette[i][1]);
        vga_dac_col_w(palette[i][2]);
    }
}

void vga_put_pixel(vga_info_t *info, u32 x, u32 y, u8 color) {
    if (x >= info->width || y >= info->height)
        return;
    if (info->mode == 0x13)
        info->base_addr[y * info->width + x] = color;
    else if (info->mode == 0x3) {
        int offset = (y * info->width + x) * 2;
        info->base_addr[offset] = '\0';
        info->base_addr[offset + 1] = color;
    }
}

void vga_fill_rect(vga_info_t *info, u32 x, u32 y, u32 width, u32 height,
                   u8 color) {
    for (u32 x_new = x; x_new < width + x; ++x_new)
        for (u32 y_new = y; y_new < height + y; ++y_new)
            vga_put_pixel(info, x_new, y_new, color);
}

void vga_scroll(vga_info_t *info, u32 y_pixels, u8 bg) {
    memcpy(info->base_addr, info->base_addr + (y_pixels * info->width),
           (info->height - y_pixels) * info->width);
    vga_fill_rect(info, 0, info->height - y_pixels, info->width, y_pixels, bg);
}

void vga_put_char(vga_info_t *info, u8 *font, size char_index, u32 char_width,
                  u32 char_height, u32 x, u32 y, u32 color_fg, u32 color_bg) {
    if (info->mode == 0x13) {
        u8(*fnt)[char_width] = (u8(*)[char_width])font;
        for (u32 row = 0; row < char_height; row++) {
            unsigned char byte = fnt[char_index][row];
            for (u32 col = 0; col < char_width; col++) {
                int bit = (byte >> (7 - col)) & 1;
                int px = x + col;
                int py = y + row;
                vga_put_pixel(info, px, py, bit ? color_fg : color_bg);
            }
        }
    } else if (info->mode == 0x3) {
        int offset = (y * info->width + x) * 2;
        info->base_addr[offset] = char_index;
        info->base_addr[offset + 1] = color_fg | color_bg;
    }
}

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
    outb(VGA_REG_SCREEN_CTRL, 14);
    int offset = inb(VGA_REG_SCREEN_DATA) << 8;
    outb(VGA_REG_SCREEN_CTRL, 15);
    offset += inb(VGA_REG_SCREEN_DATA);
    return offset * 2;
}

void vga_set_cursor_offset(int offset) {
    // Similar to get_cursor_offset, but instead of reading we write data
    offset /= 2;
    outb(VGA_REG_SCREEN_CTRL, 14);
    outb(VGA_REG_SCREEN_DATA, (u8)(offset >> 8));
    outb(VGA_REG_SCREEN_CTRL, 15);
    outb(VGA_REG_SCREEN_DATA, (u8)(offset & 0xff));
}

int vga_get_offset(int col, int row) { return 2 * (row * VGA_MAX_COLS + col); }

static int print_char(char c, int col, int row, char attr) {
    u8 *screen = (u8 *)VGA_VID_ADDR;
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
            memcpy((u8 *)(vga_get_offset(0, i - 1) + VGA_VID_ADDR),
                   (u8 *)(vga_get_offset(0, i) + VGA_VID_ADDR),
                   VGA_MAX_COLS * 2);

        char *last_line =
            (char *)(vga_get_offset(0, VGA_MAX_ROWS - 1) + (u8 *)VGA_VID_ADDR);

        for (int i = 0; i < VGA_MAX_COLS * 2; ++i)
            last_line[i] = 0;
        offset -= 2 * VGA_MAX_COLS;
    }

    vga_set_cursor_offset(offset);
    return offset;
}

void vga_clear_screen(vga_info_t *info, u8 color) {
    vga_fill_rect(info, 0, 0, info->width, info->height, color);
}

int get_offset_row(int offset) { return offset / (2 * VGA_MAX_COLS); }
int get_offset_col(int offset) {
    return (offset - (get_offset_row(offset) * 2 * VGA_MAX_COLS)) / 2;
}
