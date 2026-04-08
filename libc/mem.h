#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void memcpy(void *dest, void *source, int nbytes);
void memset(void *dest, uint8_t val, uint32_t len);
int memcmp(const void *buf1, const void *buf2, uint32_t count);
void *memmove(void *dest, const void *src, size_t n);

void kmalloc_init(void *st, uint32_t size);
void *kmalloc(uint32_t size);
void *kcalloc(uint32_t n, uint32_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, uint32_t size);

#endif
