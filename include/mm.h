#ifndef MM_H
#define MM_H

void kmalloc_init(void *st, unsigned int size);
void *kmalloc(unsigned int size);
void *kcalloc(unsigned int n, unsigned int size);
void kfree(void *ptr);
void *krealloc(void *ptr, unsigned int size);

#endif // MM_H
