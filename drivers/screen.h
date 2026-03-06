#ifndef SCREEN_H
#define SCREEN_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4
#define RED_ON_BLACK 0x04
#define WHITE_ON_RED 0x4f

enum VGAForegroundColors{
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

enum VGABackgroundColors{
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

enum BoxChars {
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

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

/* Public kernel API */
void clear_screen();
void kprint_at(char *message, int col, int row);
void kprint_at_attr(char *message, int col, int row, char attr);
void kprint(char *message);
void kprint_char(char c);
void kprint_attr(char *message, char attr);
void kprint_backspace();
int get_cursor_offset();
void set_cursor_offset(int offset);
int get_offset(int col, int row);

#endif
