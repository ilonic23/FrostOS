#include "../cpu/isr.h"
#include "../cpu/ports.h"
#include <stdint.h>

static uint8_t cur_scancode = 0;
static uint8_t ex_scancode = 0;
static uint8_t status = 0;

#define OBF (1u << 0)

#define SHIFT (1u << 0)
#define CTRL (1u << 1)
#define ALT (1u << 2)

#define IS status &

uint8_t kb_cur_scancode() { return cur_scancode; }
uint8_t kb_cur_ex_scancode() { return ex_scancode; }
uint8_t kb_cur_status() { return status; }
int kb_is_shift() { return IS SHIFT; }
int kb_is_ctrl() { return IS CTRL; }
int kb_is_alt() { return IS ALT; }

// Non-Shifted keys
static const char keymap[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' ',
};

// Shifted keys
static const char keymap_shift[128] = {
    0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
    'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' ',
};

char kb_receive_char(int halt) {
    while (halt && cur_scancode == 0)
        asm volatile("hlt");

    char c = 0;
    if (cur_scancode < 128)
        c = IS SHIFT ? keymap_shift[cur_scancode] : keymap[cur_scancode];
    cur_scancode = 0;
    return c;
}

void read_scancode() {
    if (port_byte_in(0x64) & OBF)
        cur_scancode = port_byte_in(0x60);
}

static uint8_t expect_ex = 0;

void kb_callback(registers_t *regs) {
    (void)regs;
    uint8_t byte = port_byte_in(0x60);

    if (byte == 0xE0) {
        expect_ex = 1;
        port_byte_out(0x20, 0x20);
        return;
    }

    cur_scancode = byte;
    ex_scancode = expect_ex ? byte : 0;
    expect_ex = 0;

    if (cur_scancode == 0x2A || cur_scancode == 0x36) // L/R SHIFT down
        status |= SHIFT;
    else if (cur_scancode == 0x1D) // L/R CTRL down
        status |= CTRL;
    else if (cur_scancode == 0x38) // L/R ALT down
        status |= ALT;
    else if (cur_scancode == 0xAA || cur_scancode == 0xB6) // L/R SHIFT up
        status &= ~SHIFT;
    else if (cur_scancode == 0x9D) // L/R CTRL up
        status &= ~CTRL;
    else if (cur_scancode == 0xB8) // L/R ALT up
        status &= ~ALT;

    port_byte_out(0x20, 0x20);
}

void init_keyboard() { register_interrupt_handler(IRQ1, &kb_callback); }
