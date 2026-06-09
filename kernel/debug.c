#include "debug.h"
#include "../cpu/ports.h"
#include "../libc/ctype.h"
#include "../libc/stdlib.h"
#include <stdarg.h>

#ifdef KERNEL_DEBUG

static void qemu_putchar(char c) { port_byte_out(0xE9, c); }

void kernel_trace(char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;

            // ---- %ld, %lu, %lx ----
            if (*format == 'l') {
                format++;

                if (*format == 'd') {
                    int64_t v = va_arg(args, int64_t);
                    char buf[32];
                    itoa(v, buf, 10);
                    for (char *p = buf; *p; p++)
                        qemu_putchar(*p);
                } else if (*format == 'u') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 10);
                    for (char *p = buf; *p; p++)
                        qemu_putchar(*p);
                } else if (*format == 'x') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 16);
                    for (char *p = buf; *p; p++)
                        qemu_putchar(*p);
                } else if (*format == 'o') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 8);
                    for (char *p = buf; *p; p++)
                        qemu_putchar(*p);
                }

                format++;
                continue;
            }

            // ---- %d ----
            if (*format == 'd') {
                int32_t v = va_arg(args, int32_t);
                char buf[32];
                itoa(v, buf, 10);
                for (char *p = buf; *p; p++)
                    qemu_putchar(*p);
            }
            // ---- %u ----
            else if (*format == 'u') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 10);
                for (char *p = buf; *p; p++)
                    qemu_putchar(*p);
            }
            // ---- %x ----
            else if (*format == 'x') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 16);
                for (char *p = buf; *p; p++)
                    qemu_putchar(*p);
            }
            // ---- %X ----
            else if (*format == 'X') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 16);
                for (char *p = buf; *p; p++)
                    qemu_putchar(toupper(*p));
            }
            // ---- %o ----
            else if (*format == 'o') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 8);
                for (char *p = buf; *p; p++)
                    qemu_putchar(*p);
            }
            // ---- %f ----
            else if (*format == 'f') {
                double v = va_arg(args, double);
                char buf[64];
                ftoa(v, buf, 10, 6);
                for (char *p = buf; *p; p++)
                    qemu_putchar(*p);
            } else if (*format == 'p') {
                void *v = va_arg(args, void *);
                char buf[32];
                qemu_putchar('0');
                qemu_putchar('x');
                itoa((uintptr_t)v, buf, 16);
                for (char *p = buf; *p; p++)
                    qemu_putchar(*p);
            }
            // ---- %c ----
            else if (*format == 'c') {
                char v = va_arg(args, int);
                qemu_putchar(v);
            }
            // ---- %s ----
            else if (*format == 's') {
                char *v = va_arg(args, char *);
                for (char *p = v; *p; p++)
                    qemu_putchar(*p);
            }
            // --- %% ---
            else if (*format == '%') {
                qemu_putchar('%');
            }

            format++;
        } else {
            qemu_putchar(*format++);
        }
    }

    va_end(args);
}

#endif
