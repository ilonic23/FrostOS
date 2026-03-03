#include "./ctype.h"

int islower(int c) {
    return (c >= 'a' && c <= 'z');
}

int isupper(int c) {
    return (c >= 'A' && c <= 'Z');
}

int isalpha(int c) {
    return islower(c) || isupper(c);
}

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isascii(int c) {
    return c < 128;
}

int iscntrl(int c) {
    return (c >= 0 && c < 32) || c == 127;
}

int ispunct(int c) {
    return (c > ' ' && c < '0') || (c > '9' && c < 'A') || (c > 'Z' && c < 'a') || (c > 'z' && c < 127);
}

int isgraph(int c) {
    return isalnum(c) || ispunct(c);
}

int isprint(int c) {
    return isgraph(c);
}

int isxdigit(int c) {
    return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int isspace(int c) {
    return c == '\t' || c == '\v' || c == '\n' || c == 0x0C || c == 0x0D || c == ' ';
}

int toupper(int c) {
    return islower(c) ? c^0x20 : c;
}

int tolower(int c) {
    return isupper(c) ? c^0x20 : c;
}

int toascii(int c) {
    return c & 0x7f;
}