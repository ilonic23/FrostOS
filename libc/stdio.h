/*
FrostOS implementation of stdio.h
- Made by Ilonic 2025
*/
#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stddef.h>

int sprintf(char *s, const char *format, ...);
void printf(const char *format, ...);

#endif
