#ifndef KOR_CLOCK_CLOCK_H
#define KOR_CLOCK_CLOCK_H

#include <kor/types.h>

typedef struct {
    unsigned int year : 12;
    unsigned int month : 4;
    unsigned int day : 5;
    unsigned int hours : 5;
    unsigned int minutes : 6;
    unsigned int seconds : 6;
    unsigned int milliseconds : 10;
} __attribute__((packed)) datetime_t;

#endif // KOR_CLOCK_CLOCK_H
