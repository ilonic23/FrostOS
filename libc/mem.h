#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

void memcpy(uint8_t *source, uint8_t *dest, int nbytes);
void memset(void *dest, uint8_t val, uint32_t len);
int memcmp(const void *buf1, const void *buf2, uint32_t count);
void *memmove(void *dest, const void *src, size_t n);

/* At this stage there is no 'free' implemented. */
uint32_t kmalloc(size_t size, int align, uint32_t *phys_addr);

#endif
