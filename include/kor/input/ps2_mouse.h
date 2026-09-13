#ifndef KOR_INPUT_MOUSE_H
#define KOR_INPUT_MOUSE_H

#include <kor/types.h>

// buttons bit 0: LMB, 1: MMB, 2: RMB
typedef void (*mouse_event_t)(i16 x, i16 y, u8 buttons);

void mouse_init();
void mouse_set_event(mouse_event_t event);

#endif // KOR_INPUT_MOUSE_H
