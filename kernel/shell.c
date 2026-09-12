#include "../drivers/display/display.h"
#include "../drivers/keyboard.h"
#include "../drivers/pci.h"
#include "../drivers/storage/ata.h"
#include "../libc/ctype.h"
#include "../libc/mem.h"
#include "../libc/rand.h"
#include "../libc/stdio.h"
#include "../libc/stdlib.h"
#include "../libc/string.h"
#include "debug.h"
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

void getline(char *to, char echo, uint32_t max_len) {
    uint32_t input = 0;
    char key = 0;
    char *start = to;

    while (1) {
        key = kb_receive_char(1);

        if (key == '\n')
            break;
        if (key == '\0')
            continue;

        if (key == '\b') {
            if (input > 0) {
                to--;
                input--;
                if (echo)
                    printf("\b");
            }
            continue;
        }

        if ((uint32_t)(to - start) < max_len - 1) {
            *to++ = key;
            input++;
            if (echo)
                printf("%c", key);
        }
    }

    *to = '\0';
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

void fbeep(uint32_t freq, uint32_t ms) {
    asm volatile("mov $3006, %%eax;"
                 "mov %0, %%ebx;"
                 "int $0x99;"
                 :
                 : "r"(freq)
                 : "eax", "ebx");
    sleep(ms);
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
            printf("Address: 0x%lx Size: %uB Length: %luKB Type: ", entry.addr,
                   entry.size, entry.len / 1024);
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
        ata_drive_t *drive = &get_kernel_globals()->drives[i];

        printf("Drive: %d\n\n", i);
        char serial_num[21];
        ata_str_to_c(serial_num, &drive->identify_vals[10], 10);
        printf("Serial number: %s\n", serial_num);
        char firmware[9];
        ata_str_to_c(firmware, &drive->identify_vals[23], 4);
        printf("Firmware revision: %s\n", firmware);
        char model[41];
        ata_str_to_c(model, &drive->identify_vals[27], 20);
        printf("Model: %s\n", model);
        printf("Size: %u KB ", ata_get_lba28_sects(drive) / 2);
        printf("Type: ");
        uint8_t ata = drive->flags & (1u << 1);
        uint8_t atapi = drive->flags & (1u << 2);
        uint8_t sata = drive->flags & (1u << 3);
        if (ata)
            printf("ATA IDE");
        else if (atapi)
            printf("ATA ATAPI");
        else if (sata)
            printf("ATA SATA");
        else
            printf("None");
        printf("\n\n");
    }
}

void help() {
    display_set_foreground(DISPLAY_COLOR(0, 255, 0));
    printf("--- %s Standard Shell Help ---\n",
           get_kernel_globals()->kernel_name);
    printf(
        "echo      - output a line of text\n"
        "power     - control the power of the machine\n"
        "beep      - output a sound with the PC Speaker\n"
        "rand      - output a random number\n"
        "date      - output current date\n"
        "time      - output current time\n"
        "clear     - clear screen\n"
        "uptime    - output system uptime\n"
        "osinfo    - output information about the OS\n"
        "cpuinfo   - output information about the CPU\n"
        "meminfo   - output how much memory is installed\n"
        "memmap    - output how BIOS mapped the memory\n"
        "driveinfo - output information about the installed drives\n"
        "pciinfo   - list PCI devices\n"
        "colors    - output some colors. in mode 13h you'll see them\n"
        "help      - output help\n"
        "\nSome commands may have arguments. To check for their existence, add "
        "-h\n");
    display_set_foreground(DISPLAY_COLOR(255, 255, 255));
}

void colors() {
    for (int i = 0; i < 256; ++i) {
        display_fill_rect(i, 0, 1, display_get_width(), VGA_COLOR(i));
    }
    while (kb_receive_char(1) != '\b')
        ;
}

void pciinfo() {
    for (int i = 0; i < get_kernel_globals()->pci_dev_count; ++i) {
        uint8_t bus =
            (uint8_t)((get_kernel_globals()->pci_devs[i] >> 8) & 0xFF);
        uint8_t slot = (uint8_t)(get_kernel_globals()->pci_devs[i] & 0xFF);
        pci_base_device_header_t header =
            pci_get_base_device_header(bus, slot, 0x00);
        printf("Vendor: 0x%X Device: 0x%X Class: 0x%X Subclass: 0x%X ProgIF: "
               "0x%X\n",
               header.vendor, header.device_id, header.class_code,
               header.subclass, header.progif);
    }
}

void shell() {
    char *input = kmalloc(512);
    char *arg_buffer = kmalloc(512);
    char **argv = kmalloc(sizeof(char *) * 8);
    int argc;

    for (int i = 0; i < 10; ++i) {
        fbeep(31 * 4, 20);
        fbeep(49 * 4, 20);
    }

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
        else if (strcmp(argv[0], "help") == 0)
            help();
        else if (strcmp(argv[0], "colors") == 0)
            colors();
        else if (strcmp(argv[0], "pciinfo") == 0)
            pciinfo();
        else {
            display_set_foreground(DISPLAY_COLOR(255, 0, 0));
            printf("Unknown file or command: %s\n", argv[0]);
            display_set_foreground(DISPLAY_COLOR(255, 255, 255));
        }
    }
}
