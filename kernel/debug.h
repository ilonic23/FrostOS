#ifndef DEBUG_H
#define DEBUG_H

#ifdef KERNEL_DEBUG
void kernel_trace(char *format, ...);
#define trace(fmt, ...)                                                        \
    kernel_trace("[TRACE] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define trace_empty(fmt, ...) kernel_trace(fmt, ##__VA_ARGS__)
#else
#define trace(fmt, ...) ((void)0)
#define trace_empty(fmt, ...) ((void)0)
#endif

#endif
