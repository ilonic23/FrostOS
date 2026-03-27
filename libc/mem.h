#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

void memcpy(void *source, void *dest, int nbytes);
void memset(void *dest, uint8_t val, uint32_t len);
int memcmp(const void *buf1, const void *buf2, uint32_t count);
void *memmove(void *dest, const void *src, size_t n);

#endif
