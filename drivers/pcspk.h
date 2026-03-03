#ifndef PCSPK_H
#define PCSPK_H

#include <stdint.h>

void speaker_set_frequency(uint32_t nFrequence);
void speaker_stop();
void speaker_beep();

#endif