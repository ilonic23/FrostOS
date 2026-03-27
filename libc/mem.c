#include "mem.h"

void memory_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

void memcpy(void *source, void *dest, int nbytes) {
    for (int i = 0; i < nbytes; i++)
        *((uint8_t *)dest + i) = *((uint8_t *)source + i);
}

void memset(void *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

int memcmp(const void *buf1, const void *buf2, uint32_t count) {
    if (!count) return 0;

    while (--count && *(char *)buf1 == *(char *)buf2) {
        buf1 = (char *)buf1 + 1;
        buf2 = (char *)buf2 + 1;
    }

    return *((unsigned char *)buf1) - *((unsigned char *)buf2);
}

void *memmove(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;

    if (d == s) { // If source and destination are the same, no copy needed
        return dest;
    }

    if (d < s) { // Destination is before source, copy forwards
        while (n--) {
            *d++ = *s++;
        }
    } else { // Destination is after source, or overlaps in a way that requires backward copy
        d += n; // Move pointers to the end of the blocks
        s += n;
        while (n--) {
            *--d = *--s; // Copy backwards
        }
    }
    return dest;
}
