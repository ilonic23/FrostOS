#ifndef STRINGS_H
#define STRINGS_H

#include <stdint.h>

void int_to_ascii(int n, char str[]);
void hex_to_ascii(int n, char str[]);
void reverse(char s[]);
int strlen(char s[]);
void backspace(char s[]);
void append(char s[], char n);
int strcmp(char s1[], char s2[]);
// char *itoa(int value, char *str, int base);
void strfmt(const char *format, char *dest, uint64_t max, void **args);

#endif
