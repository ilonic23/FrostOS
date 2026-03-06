#ifndef PCSPK_H
#define PCSPK_H

#include <stdint.h>

uint32_t speaker_get_freq();
uint8_t speaker_active();
void speaker_set_frequency(uint32_t nFrequence);
void speaker_stop();
void speaker_beep();

#endif
