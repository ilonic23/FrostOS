#include "../drivers/display/display.h"
#include "../drivers/keyboard.h"
#include "../libc/ctype.h"
#include "../libc/mem.h"
#include "../libc/rand.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "kernel.h"
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

uint64_t get_ms() {
    uint32_t low, high;
    asm volatile("mov $3001, %%eax;"
                 "int $0x99;"
                 : "=a"(low), "=b"(high)
                 :
                 : "memory");
    return ((uint64_t)high << 32 | low);
}

void sleep(uint64_t ms) {
    uint64_t start = get_ms();
    uint64_t current = get_ms();
    while (current - start < ms)
        current = get_ms();
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

void beep() {
    asm volatile("mov $3005, %eax;"
                 "int $0x99;");
    sleep(100);
    asm volatile("mov $3007, %eax;"
                 "int $0x99;");
}

void rand() {
    char temp[32] = {0};
    pcg32_init(get_ms());
    sprintf(temp, "%u", pcg32());
    for (int i = 0; i < 32; ++i) {
        display_set_foreground(DISPLAY_COLOR(
            pcg32_lim(40, 255), pcg32_lim(40, 255), pcg32_lim(40, 255)));
        printf("%c", temp[i]);
    }
    display_set_foreground(DISPLAY_COLOR(255, 255, 255));
    printf("\n");
}

void date() {
    uint32_t years, months, days;
    asm volatile("mov $3030, %%eax;"
                 "int $0x99;"
                 : "=a"(years), "=b"(months), "=c"(days)
                 :
                 : "memory");
    printf("Date: %u.%u.%u\n", years, months, days);
}

void time() {
    uint32_t hours, minutes, seconds, milliseconds;
    asm volatile("mov $3031, %%eax;"
                 "int $0x99;"
                 : "=a"(hours), "=b"(minutes), "=c"(seconds), "=d"(milliseconds)
                 :
                 : "memory");
    printf("Time: %u:%u:%u:%u\n", hours, minutes, seconds, milliseconds);
}

void clear() {
    asm volatile("mov $3010, %eax;"
                 "int $0x99;");
}

void uptime() {
    uint64_t uptime = get_ms();
    printf("Uptime:\n %lu Hours %lu Minutes %lu Seconds %lu "
           "Milliseconds\n",
           uptime / (1000 * 60 * 60), (uptime / (1000 * 60)) % 60,
           (uptime / 1000) % 60, uptime % 1000);
    printf("Raw:\n %lu ms\n", uptime);
}

void osinfo() {
    printf("OS: %s\nVersion: %s\nCodename: %s\n",
           get_kernel_globals()->kernel_name,
           get_kernel_globals()->kernel_version,
           get_kernel_globals()->kernel_codename);
}

void cpuinfo() {
    if (get_kernel_globals()->cpuid_supported)
        printf("Vendor: %s\nFull name: %s\n",
               get_kernel_globals()->cpu_manufacturer,
               get_kernel_globals()->cpu_full_name);
    else
        printf("CPUID instruction not supported.\n");
}

void meminfo() {
    multiboot_info_t *mbi = get_kernel_globals()->multiboot_addr;
    printf("Lower memory: %lu KB\n", mb_get_lower_mem(mbi) / 1024);
    printf("Upper memory %lu KB\n", mb_get_upper_mem(mbi) / 1024);
    printf("Total memory %lu KB\n", mb_get_mem(mbi) / 1024);
}

void memmap() {
    multiboot_info_t *mbi = get_kernel_globals()->multiboot_addr;
    if (mbi->flags & (1u << 6))
        for (uint32_t i = 0; i < mbi->mmap_length;
             i += sizeof(multiboot_mmap_entry_t)) {
            multiboot_mmap_entry_t entry =
                *(multiboot_mmap_entry_t *)(uintptr_t)(mbi->mmap_addr + i);
            printf("Entry: %d\n", i);
            printf("Address: 0x%lx Size: %uB Length: %luB Type: ", entry.addr,
                   entry.size, entry.len);
            switch (entry.type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                printf("Available");
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                printf("Reserved");
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                printf("ACPI reclaimable");
                break;
            case MULTIBOOT_MEMORY_NVS:
                printf("NVS");
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                printf("BADRAM");
                break;
            default:
                printf("Unknown");
                break;
            }
            printf("\n");
        }
    else {
        printf("Multiboot memory maps unavailable.\n");
    }
}

void driveinfo() {
    for (int i = 0; i < get_kernel_globals()->drives_count; ++i) {
        printf("Drive %d:\n", i);
        printf("Size: %u KB ",
               drive_lba28_sects(&get_kernel_globals()->drives[i]) / 2);
        printf("Type: ");
        switch (get_kernel_globals()->drives[i].type) {
        case DRIVE_NULL:
            printf("None");
            break;
        case DRIVE_ATA:
            printf("ATA IDE");
            break;
        case DRIVE_ATA_ATAPI:
            printf("ATA ATAPI");
            break;
        case DRIVE_ATA_SATA:
            printf("ATA SATA");
            break;
        default:
            printf("Unsupported");
            break;
        }
        printf("\n");
    }
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
        else if (strcmp(argv[0], "beep") == 0)
            beep();
        else if (strcmp(argv[0], "rand") == 0)
            rand();
        else if (strcmp(argv[0], "date") == 0)
            date();
        else if (strcmp(argv[0], "time") == 0)
            time();
        else if (strcmp(argv[0], "clear") == 0)
            clear();
        else if (strcmp(argv[0], "uptime") == 0)
            uptime();
        else if (strcmp(argv[0], "osinfo") == 0)
            osinfo();
        else if (strcmp(argv[0], "cpuinfo") == 0)
            cpuinfo();
        else if (strcmp(argv[0], "meminfo") == 0)
            meminfo();
        else if (strcmp(argv[0], "memmap") == 0)
            memmap();
        else if (strcmp(argv[0], "driveinfo") == 0)
            driveinfo();
        else {
            display_set_foreground(DISPLAY_COLOR(255, 0, 0));
            printf("Unknown file or command: %s\n", argv[0]);
            display_set_foreground(DISPLAY_COLOR(255, 255, 255));
        }
    }
}
