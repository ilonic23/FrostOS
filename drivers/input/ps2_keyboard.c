#include <kor/input/ps2.h>
#include <kor/input/ps2_keyboard.h>
#include <kor/ll/isr.h>
#include <kor/ll/port.h>
#include <kor/types.h>

#define SHIFT (1u << 0)
#define CTRL (1u << 1)
#define ALT (1u << 2)
#define CAPS (1u << 3)

#define IS status &

static int scancode;
static int scancode_ex;
static u8 status;

u8 kb_get_scancode() { return scancode; }
u8 kb_get_scancode_ex() { return scancode_ex; }
u8 kb_get_status() { return status; }
int kb_is_shift() { return IS SHIFT; }
int kb_is_ctrl() { return IS CTRL; }
int kb_is_alt() { return IS ALT; }

static inline u8 ps2_kb_read() {
    if (ps2_status_read() & 1u)
        return ps2_data_read();
    return 0;
}

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

int kb_receive(int halt) {
    while (halt && scancode == 0)
        asm volatile("hlt");

    int c = 0;
    if (scancode < 128)
        c = IS SHIFT ? keymap_shift[scancode] : keymap[scancode];
    if (c == 'E' && IS CTRL)
        c = -1; // EOF
    scancode = 0;
    return c;
}

static int expect_ex;

void kb_callback(registers_t *regs) {
    (void)regs;
    u8 byte = ps2_kb_read();

    if (byte == 0xE0) {
        expect_ex = 1;
        return;
    }

    scancode = byte;
    scancode_ex = expect_ex ? byte : 0;
    expect_ex = 0;

    if (scancode == 0x2A || scancode == 0x36) // L/R SHIFT down
        status |= SHIFT;
    else if (scancode == 0x1D) // L/R CTRL down
        status |= CTRL;
    else if (scancode == 0x38) // L/R ALT down
        status |= ALT;
    else if (scancode == 0xAA || scancode == 0xB6) // L/R SHIFT up
        status &= ~SHIFT;
    else if (scancode == 0x9D) // L/R CTRL up
        status &= ~CTRL;
    else if (scancode == 0xB8) // L/R ALT up
        status &= ~ALT;

    if (scancode == 0x3A) { // CapsLock down
        if (!(IS CAPS))
            status |= CAPS;
        else
            status &= ~CAPS;
    }

    if (IS CAPS) // toggle CapsLock led
        ps2_kb_send_cmd(0xED, 0b00000100);
}

void kb_init() { register_interrupt_handler(IRQ1, kb_callback); }
