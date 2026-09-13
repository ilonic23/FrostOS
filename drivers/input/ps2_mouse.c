#include <kor/input/ps2.h>
#include <kor/input/ps2_mouse.h>
#include <kor/ll/isr.h>
#include <kor/types.h>

static mouse_event_t current_event;

void mouse_wait(int write) {
    u32 timeout = 100000;

    if (write) {
        while (timeout--)
            if (!(ps2_status_read() & 2))
                return;
    } else {
        while (timeout--)
            if (!(ps2_status_read() & 1))
                return;
    }
}

static inline void mouse_write(u8 byte) {
    mouse_wait(1);
    ps2_command_write(0xD4);
    mouse_wait(1);
    ps2_data_write(byte);
}

static inline u8 mouse_read() {
    mouse_wait(0);
    return ps2_data_read();
}

void mouse_callback(registers_t *regs) {
    (void)regs;
    u8 packets[4];
    for (int i = 0; i < 3; ++i)
        packets[i] = ps2_data_read();
    if (current_event != (void *)0) {
        u8 buttons = 0;
        if (packets[0] & 1)
            buttons |= 1; // LMB
        if (packets[0] & 4)
            buttons |= 2; // MMB
        if (packets[0] & 2)
            buttons |= 4; // RMB
        i16 x = packets[1];
        i16 y = packets[2];
        if (packets[0] & 16)
            x = -x;
        if (packets[0] & 32)
            y = -y;
        current_event(x, y, buttons);
    }
}

void mouse_set_event(mouse_event_t event) { current_event = event; }

void mouse_init() {
    register_interrupt_handler(IRQ12, mouse_callback);
    // enable aux mouse
    mouse_wait(1);
    ps2_command_write(0xA8);

    // enable interrupt
    mouse_wait(1);
    ps2_command_write(0x20);
    mouse_wait(0);
    u8 in = ps2_data_read();
    in |= 2;
    in &= ~0x20;
    mouse_wait(1);
    ps2_command_write(0x60);
    mouse_wait(1);
    ps2_data_write(in);

    // set defaults
    mouse_write(0xF6);
    mouse_read(); // ACK (acknowldege: 0xFA)

    // enable packet streaming
    mouse_write(0xF4);
    mouse_read(); // ACK
}
