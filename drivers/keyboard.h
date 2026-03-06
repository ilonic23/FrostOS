#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

uint8_t get_cur_scancode();
char keyboard_receive_key(char halt);
void getline(char *to, char echo, uint32_t max_len);
void init_keyboard();


#endif
