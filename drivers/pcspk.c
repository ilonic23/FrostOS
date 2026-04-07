#include "pcspk.h"
#include "../cpu/ports.h"
#include "../cpu/timer.h"
#include <stdint.h>

static uint32_t cur_freq = 0;

uint32_t speaker_get_freq() { return cur_freq; }

// Set PC speaker frequency to play
void speaker_set_frequency(uint32_t nFrequence) {
    cur_freq = nFrequence;
    uint32_t Div;
    uint8_t tmp;

    // Set the PIT to the desired frequency
    Div = 1193180 / nFrequence;
    port_byte_out(0x43, 0xb6);
    port_byte_out(0x42, (uint8_t)(Div));
    port_byte_out(0x42, (uint8_t)(Div >> 8));

    // And play the sound using the PC speaker
    tmp = port_byte_in(0x61);
    if (tmp != (tmp | 3)) {
        port_byte_out(0x61, tmp | 3);
    }
}

// Stop playing sound
void speaker_stop() {
    uint8_t tmp = port_byte_in(0x61) & 0xFC;
    port_byte_out(0x61, tmp);
}

// Make a beep
void speaker_beep() {
    speaker_set_frequency(440);
    sleep_ms(100);
    speaker_stop();
}
