#include "mem.h"
#include "./stdio.h"

void memory_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for (; len != 0; len--)
        *temp++ = val;
}

void memcpy(void *dest, void *source, int nbytes) {
    for (int i = 0; i < nbytes; i++)
        *((uint8_t *)dest + i) = *((uint8_t *)source + i);
}

void memset(void *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for (; len != 0; len--)
        *temp++ = val;
}

int memcmp(const void *buf1, const void *buf2, uint32_t count) {
    if (!count)
        return 0;

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
    } else { // Destination is after source, or overlaps in a way that requires
             // backward copy
        d += n; // Move pointers to the end of the blocks
        s += n;
        while (n--) {
            *--d = *--s; // Copy backwards
        }
    }
    return dest;
}

typedef struct __attribute__((packed)) mem_block {
    uint8_t status; // bit 0 set - free, else used
    uint32_t size;
    struct mem_block *next;
} mem_block_t;

static void *start;

void kmalloc_init(void *st, uint32_t size) {
    start = st;
    mem_block_t block = {.status = 0x01, .size = size, .next = 0};
    memcpy(st, &block, sizeof(block));
}

void *kmalloc(uint32_t size) {
    mem_block_t *next_free = (mem_block_t *)start;
    while (1) {
        if (next_free->status & (1u << 0)) {
            if (next_free->size >= size + sizeof(mem_block_t))
                break;
        }
        if (next_free->next == 0)
            return NULL;
        next_free = (mem_block_t *)(uintptr_t)next_free->next;
    }

    mem_block_t *cur_block = next_free;
    uint32_t old_size = cur_block->size;
    mem_block_t *old_next = cur_block->next;

    cur_block->status &= ~(1u << 0);
    cur_block->size = size;

    uint32_t remaining = old_size - size - sizeof(mem_block_t);
    if (old_size >= size + sizeof(mem_block_t) + 4) {
        mem_block_t *new_free =
            (mem_block_t *)((uint8_t *)(cur_block + 1) + size);
        mem_block_t new_free_block = {
            .status = 0x01, .size = remaining, .next = old_next};
        memcpy(new_free, &new_free_block, sizeof(new_free_block));
        cur_block->next = new_free;
    } else {
        cur_block->size = old_size;
        cur_block->next = old_next;
    }

    return (void *)(cur_block + 1);
}

void *kcalloc(uint32_t n, uint32_t size) {
    void *ptr = kmalloc(n * size);
    if (ptr == NULL)
        return NULL;
    memset(ptr, 0, n * size);

    return ptr;
}

void kfree(void *ptr) {
    mem_block_t *block = ((mem_block_t *)ptr - 1);
    block->status |= (1u << 0);

    // Merge free blocks
    while (block->next != 0) {
        mem_block_t *next = (mem_block_t *)(uintptr_t)block->next;
        if (!(next->status & (1u << 0)))
            break;

        block->size += sizeof(mem_block_t) + next->size;
        block->next = next->next;
    }
}

void *krealloc(void *ptr, uint32_t size) {
    if (ptr == NULL)
        return NULL;

    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    void *new_ptr = kmalloc(size);
    if (new_ptr == NULL)
        return NULL;

    mem_block_t *block = ((mem_block_t *)ptr - 1);
    uint32_t copy_size = block->size < size ? block->size : size;
    memcpy(new_ptr, ptr, copy_size);
    kfree(ptr);

    return new_ptr;
}
