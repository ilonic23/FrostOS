#include "./stdio.h"
#include "./ctype.h"
#include "../drivers/screen.h"
#include <stdint.h>
#include <stdarg.h>
#include "./stdlib.h"

void printf(const char *format, ...) {
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
                    for (char *p = buf; *p; p++) kprint_char(*p);
                }
                else if (*format == 'u') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 10);
                    for (char *p = buf; *p; p++) kprint_char(*p);
                }
                else if (*format == 'x') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 16);
                    for (char *p = buf; *p; p++) kprint_char(*p);
                }
                else if (*format == 'o') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 8);
                    for (char *p = buf; *p; p++) kprint_char(*p);
                }

                format++;
                continue;
            }

            // ---- %d ----
            if (*format == 'd') {
                int32_t v = va_arg(args, int32_t);
                char buf[32];
                itoa(v, buf, 10);
                for (char *p = buf; *p; p++) kprint_char(*p);
            }
            // ---- %u ----
            else if (*format == 'u') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 10);
                for (char *p = buf; *p; p++) kprint_char(*p);
            }
            // ---- %x ----
            else if (*format == 'x') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 16);
                for (char *p = buf; *p; p++) kprint_char(*p);
            }
            // ---- %X ----
            else if (*format == 'X') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 16);
                for (char *p = buf; *p; p++) kprint_char(toupper(*p));
            }
            // ---- %o ----
            else if (*format == 'o') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 8);
                for (char *p = buf; *p; p++) kprint_char(*p);
            }
            // ---- %f ----
            else if (*format == 'f') {
                double v = va_arg(args, double);
                char buf[64];
                ftoa(v, buf, 10, 6);
                for (char *p = buf; *p; p++) kprint_char(*p);
            } else if (*format == 'p') {
                void *v = va_arg(args, void *);
                char buf[32];
                kprint_char('0');
                kprint_char('x');
                itoa((uintptr_t)v, buf, 16);
                for (char *p = buf; *p; p++) kprint_char(*p);
            }
            // ---- %c ----
            else if (*format == 'c') {
                char v = va_arg(args, char);
                kprint_char(v);
            }
            // ---- %s ----
            else if (*format == 's') {
                char *v = va_arg(args, char *);
                for (char *p = v; *p; p++) kprint_char(*p);
            }
            // --- %% ---
            else if (*format == '%') {
                kprint_char('%');
            }

            format++;
        }
        else {
            kprint_char(*format++);
        }
    }

    va_end(args);
}

int sprintf(char *s, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *start = s;

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
                    for (char *p = buf; *p; p++) *s++ = *p;
                }
                else if (*format == 'u') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 10);
                    for (char *p = buf; *p; p++) *s++ = *p;
                }
                else if (*format == 'x') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 16);
                    for (char *p = buf; *p; p++) *s++ = *p;
                }
                else if (*format == 'o') {
                    uint64_t v = va_arg(args, uint64_t);
                    char buf[32];
                    itoa(v, buf, 8);
                    for (char *p = buf; *p; p++) *s++ = *p;
                }

                format++;
                continue;
            }

            // ---- %d ----
            if (*format == 'd') {
                int32_t v = va_arg(args, int32_t);
                char buf[32];
                itoa(v, buf, 10);
                for (char *p = buf; *p; p++) *s++ = *p;
            }
            // ---- %u ----
            else if (*format == 'u') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 10);
                for (char *p = buf; *p; p++) *s++ = *p;
            }
            // ---- %x ----
            else if (*format == 'x') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 16);
                for (char *p = buf; *p; p++) *s++ = *p;
            }
            // ---- %X ----
            else if (*format == 'X') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 16);
                for (char *p = buf; *p; p++) *s++ = toupper(*p);
            }
            // ---- %o ----
            else if (*format == 'o') {
                uint32_t v = va_arg(args, uint32_t);
                char buf[32];
                itoa(v, buf, 8);
                for (char *p = buf; *p; p++) *s++ = *p;
            }
            // ---- %f ----
            else if (*format == 'f') {
                double v = va_arg(args, double);
                char buf[64];
                ftoa(v, buf, 10, 6);
                for (char *p = buf; *p; p++) *s++ = *p;
            } else if (*format == 'p') {
                void *v = va_arg(args, void *);
                char buf[32];
                *s++ = '0';
                *s++ = 'x';
                itoa((uintptr_t)v, buf, 16);
                for (char *p = buf; *p; p++) *s++ = *p;
            }
            // ---- %c ----
            else if (*format == 'c') {
                char v = va_arg(args, char);
                *s++ = v;
            }
            // ---- %s ----
            else if (*format == 's') {
                char *v = va_arg(args, char *);
                for (char *p = v; *p; p++) *s++ = *p;
            }
            // --- %% ---
            else if (*format == '%') {
                *s++ = '%';
            }

            format++;
        }
        else {
            *s++ = *format++;
        }
    }

    *s = '\0';
    va_end(args);
    return s - start;
}
