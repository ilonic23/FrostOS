#ifndef KOR_INPUT_PS2_H
#define KOR_INPUT_PS2_H

#include <kor/ll/port.h>
#include <kor/types.h>

static inline u8 ps2_data_read() { return inb(0x60); }
static inline void ps2_data_write(u8 val) { outb(0x60, val); }
static inline u8 ps2_status_read() { return inb(0x64); }
static inline void ps2_command_write(u8 val) { outb(0x64, val); }

#endif // KOR_INPUT_PS2_H
