#include "mem.h"

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

void memory_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

void memcpy(uint8_t *source, uint8_t *dest, int nbytes) {
    for (int i = 0; i < nbytes; i++)
        *(dest + i) = *(source + i);
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

// int memcmp(const void* buf1,
//            const void* buf2,
//            size_t count)
// {
//     if(!count)
//         return(0);

//     while(--count && *(char*)buf1 == *(char*)buf2 ) {
//         buf1 = (char*)buf1 + 1;
//         buf2 = (char*)buf2 + 1;
//     }

//     return(*((unsigned char*)buf1) - *((unsigned char*)buf2));
// }


/* This should be computed at link time, but a hardcoded
 * value is fine for now. Remember that our kernel starts
 * at 0x1000 as defined on the Makefile */
uint32_t free_mem_addr = 0x10000;
/* Implementation is just a pointer to some free memory which
 * keeps growing */
uint32_t kmalloc(size_t size, int align, uint32_t *phys_addr) {
    /* Pages are aligned to 4K, or 0x1000 */
    if (align == 1 && (free_mem_addr & 0xFFFFF000)) {
        free_mem_addr &= 0xFFFFF000;
        free_mem_addr += 0x1000;
    }
    /* Save also the physical address */
    if (phys_addr) *phys_addr = free_mem_addr;

    uint32_t ret = free_mem_addr;
    free_mem_addr += size; /* Remember to increment the pointer */
    return ret;
}
