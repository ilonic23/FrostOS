/*
FrostOS implementation of stdio.h
- Made by Ilonic 2025
*/
#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>

#define NULL ((void*)0)
#define EOF -1

int sprintf(char *s, const char *format, ...);
void printf(const char *format, ...);

#endif
