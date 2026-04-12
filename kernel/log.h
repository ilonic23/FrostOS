#ifndef LOG_H
#define LOG_H

#include "../drivers/display/display.h"
#include "../libc/stdio.h"

void sys_log_msg(char *msg);
void sys_log_wrn(char *wrn);
void sys_log_err(char *err);

#endif
