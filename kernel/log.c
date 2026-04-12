#include "log.h"
#include "../drivers/display/display.h"
#include "../libc/stdio.h"

void sys_log_msg(char *msg) {
    display_set_foreground(DISPLAY_COLOR(0, 255, 255));
    printf("[MSG] %s\n", msg);
    display_set_foreground(DISPLAY_COLOR(255, 255, 255));
}

void sys_log_wrn(char *wrn) {
    display_set_foreground(DISPLAY_COLOR(255, 255, 0));
    printf("[WRN] %s\n", wrn);
    display_set_foreground(DISPLAY_COLOR(255, 255, 255));
}

void sys_log_err(char *err) {
    display_set_foreground(DISPLAY_COLOR(255, 0, 0));
    printf("[ERR] %s\n", err);
    display_set_foreground(DISPLAY_COLOR(255, 255, 255));
}
