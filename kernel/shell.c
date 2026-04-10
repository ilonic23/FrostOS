#include "../drivers/keyboard.h"
#include "../libc/ctype.h"
#include "../libc/mem.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include <stdint.h>

int split_string(char *string, char **output) {
    int words = 0;
    int in_quotes = 0;
    int space = 1; // Last seen character is a space
    char *write_ptr = output[0];

    if (string[0] == '\0')
        return 0;
    for (int i = 0; string[i]; ++i) {
        if (string[i] == '\\' && string[i + 1] == '\"') {
            *write_ptr++ = '\"';
            i++;
            space = 0;
        } else if (string[i] == '\"') {
            in_quotes = !in_quotes;
            space = 0;
        } else if (string[i] == ' ') {
            if (!in_quotes && !space) {
                *write_ptr++ = '\0';
                words++;
                output[words] = write_ptr;
                space = 1;
            } else {
                *write_ptr++ = string[i];
                space = 0;
            }
        } else {
            *write_ptr++ = string[i];
            space = 0;
        }
    }
    return words + 1;
}

int64_t stoll(const char *str) {
    int64_t result = 0;
    uint8_t sign = 0;
    if (*str == '-') {
        sign = 1;
        str++;
    }
    while (isdigit(*str)) {
        result *= 10;
        result += *str++ - '0';
    }
    if (sign)
        result *= -1;
    return result;
}

double stod(const char *str) {
    double result = 0;
    uint8_t sign = 0;
    int8_t after_dot = -1;
    if (*str == '-') {
        sign = 1;
        str++;
    }
    while (isdigit(*str) || *str == '.') {
        if (*str == '.') {
            after_dot = 0;
            str++;
        } else {
            result *= 10;
            result += *str++ - '0';
            if (after_dot != -1)
                after_dot++;
        }
    }
    for (int i = 0; i < after_dot && after_dot != -1; ++i)
        result /= 10;
    if (sign)
        result *= -1;
    return result;
}

int echo(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <str>\n", argv[0]);
        return 0;
    }

    for (int i = 1; i < argc; ++i)
        printf("%s ", argv[i]);
    return 0;
}

int power(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <args>\nFor help use --help argument.\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        printf("%s - control the power of the machine\nUsage:\n", argv[0]);
        printf("-h --help - Show this help and exit\n");
        printf("-H --halt - Halt the system\n");
        printf("-r --reboot - Reboot the system\n");
        return 0;
    }
    if (strcmp(argv[1], "--halt") == 0 || strcmp(argv[1], "-H") == 0) {
        asm volatile("mov $3004 , %eax;"
                     "int $0x99;");
        return 0;
    }
    if (strcmp(argv[1], "--reboot") == 0 || strcmp(argv[1], "-r") == 0) {
        asm volatile("mov $3003 , %eax;"
                     "int $0x99;");
        return 0;
    }

    return 0;
}

void shell() {
    char *input = kmalloc(512);
    char *arg_buffer = kmalloc(512);
    char **argv = kmalloc(sizeof(char *) * 8);
    int argc;

    while (1) {
        memset(input, 0, 512);
        memset(arg_buffer, 0, 512);
        argv[0] = arg_buffer;
        printf("\n> ");
        getline(input, 1, 512);
        argc = split_string(input, argv);
        printf("\n");

        if (strcmp(argv[0], "echo") == 0)
            echo(argc, argv);
        else if (strcmp(argv[0], "power") == 0)
            power(argc, argv);
        else
            printf("Unknown command.\n");
    }
}
