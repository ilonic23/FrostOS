#include <kstring.h>

int memcmp(const void *str1, const void *str2, unsigned int n) {
    if (!n)
        return 0;

    while (--n && *(char *)str1 == *(char *)str2) {
        str1 = (char *)str1 + 1;
        str2 = (char *)str2 + 1;
    }
    return *((unsigned char *)str1) - *((unsigned char *)str2);
}

void *memcpy(void *dest, const void *src, unsigned int n) {
    for (unsigned int i = 0; i < n; ++i)
        *((unsigned char *)dest + i) = *((unsigned char *)src + i);
    return dest;
}

void *memset(void *str, int c, unsigned n) {
    for (unsigned int i = 0; i < n; ++i)
        *((unsigned char *)str + i) = (unsigned char)c;
    return str;
}

typedef struct __attribute__((packed)) mem_block {
    unsigned char status; // bit 0 set - free, else used
    unsigned int size;
    struct mem_block *next;
} mem_block_t;

static void *start;

void kmalloc_init(void *st, unsigned int size) {
    start = st;
    mem_block_t block = {.status = 0x01, .size = size, .next = 0};
    memcpy(st, &block, sizeof(block));
}

void *kmalloc(unsigned int size) {
    mem_block_t *next_free = (mem_block_t *)start;
    while (1) {
        if (next_free->status & (1u << 0)) {
            if (next_free->size >= size + sizeof(mem_block_t))
                break;
        }
        if (next_free->next == 0)
            return (void *)0;
        next_free = (mem_block_t *)next_free->next;
    }

    mem_block_t *cur_block = next_free;
    unsigned int old_size = cur_block->size;
    mem_block_t *old_next = cur_block->next;

    cur_block->status &= ~(1u << 0);
    cur_block->size = size;

    unsigned int remaining = old_size - size - sizeof(mem_block_t);
    if (old_size >= size + sizeof(mem_block_t) + 4) {
        mem_block_t *new_free =
            (mem_block_t *)((unsigned char *)(cur_block + 1) + size);
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

void *kcalloc(unsigned int n, unsigned int size) {
    void *ptr = kmalloc(n * size);
    if (ptr == (void *)0)
        return (void *)0;
    memset(ptr, 0, n * size);

    return ptr;
}

void kfree(void *ptr) {
    mem_block_t *block = ((mem_block_t *)ptr - 1);
    block->status |= (1u << 0);

    // Merge free blocks
    while (block->next != 0) {
        mem_block_t *next = (mem_block_t *)block->next;
        if (!(next->status & (1u << 0)))
            break;

        block->size += sizeof(mem_block_t) + next->size;
        block->next = next->next;
    }
}

void *krealloc(void *ptr, unsigned int size) {
    if (ptr == (void *)0)
        return (void *)0;

    if (size == 0) {
        kfree(ptr);
        return (void *)0;
    }

    void *new_ptr = kmalloc(size);
    if (new_ptr == (void *)0)
        return (void *)0;

    mem_block_t *block = ((mem_block_t *)ptr - 1);
    unsigned int copy_size = block->size < size ? block->size : size;
    memcpy(new_ptr, ptr, copy_size);
    kfree(ptr);

    return new_ptr;
}
