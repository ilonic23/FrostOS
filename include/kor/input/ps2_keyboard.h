#ifndef KOR_INPUT_PS2_KEYBOARD_H
#define KOR_INPUT_PS2_KEYBOARD_H

#include <kor/input/ps2.h>
#include <kor/types.h>

static inline void ps2_kb_send_cmd(u8 cmd, u8 data) {
    if (ps2_status_read() & 2) {
        ps2_command_write(cmd);
        ps2_data_write(data);
    }
}

u8 kb_get_scancode();
u8 kb_get_scancode_ex();
u8 kb_get_status();
int kb_is_shift();
int kb_is_ctrl();
int kb_is_alt();

void kb_init();
int kb_receive(int halt);

#endif // KOR_INPUT_PS2_KEYBOARD_H
