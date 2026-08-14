#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

uint8_t kb_cur_scancode();
uint8_t kb_cur_ex_scancode();
uint8_t kb_cur_status();
int kb_is_shift();
int kb_is_ctrl();
int kb_is_alt();
char kb_receive_char(int halt);
void init_keyboard();

#endif
