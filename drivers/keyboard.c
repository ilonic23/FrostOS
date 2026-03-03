#include "../cpu/ports.h"
#include <stdint.h>
#include "screen.h"

char keyboard_receive_key(char halt)
{
    static int shift_pressed = 0;
    unsigned char scancode = 0;

    // Halt everything until we receive a key
    while (halt && (port_byte_in(0x64) & 1) == 0) ;
    
    scancode = port_byte_in(0x60);
    if (!scancode) {
        volatile char a = 1; char x = a / 0;
    }

    // Key release (break code)
    if (scancode & 0x80) {
        unsigned char release_code = scancode & 0x7F;
        if (release_code == 0x2A || release_code == 0x36) // левый или правый Shift
            shift_pressed = 0;
        return 0; // Ignore key releases
    }

    // Shift pressed
    if (scancode == 0x2A || scancode == 0x36) { // Left/right Shift
        shift_pressed = 1;
        return 0;
    }

    // Non-Shifted keys
    static const char keymap[128] = {
        0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
        0, '\\','z','x','c','v','b','n','m',',','.','/', 0,
        '*',0,' ',
    };

    // Shifted keys
    static const char keymap_shift[128] = {
        0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
        '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
        0, 'A','S','D','F','G','H','J','K','L',':','"','~',
        0, '|','Z','X','C','V','B','N','M','<','>','?', 0,
        '*',0,' ',
    };

    char c = shift_pressed ? keymap_shift[scancode] : keymap[scancode];
    return c;
}

void getline(char *to, char echo, uint32_t max_len) {
    char key = 0;
    char temp[2];
    char *start = to;

    while (1) {
        key = keyboard_receive_key(1);
        if (key == '\n') break;
        if (key == '\0') {
        	continue;
        } // ignore

        if ((uint32_t)(to - start) < max_len - 1) {
            *to++ = key;
            if (key == '\b') {
            	*to--;
            	*to--;
            }
            if (echo) {
                temp[0] = key;
                temp[1] = '\0';
                kprint(temp);
            }
        }
    }

    *to = '\0';
}
