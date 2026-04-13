#include "../drivers/clock.h"
#include "../drivers/storage/drive.h"
#include "../multiboot/multiboot.h"
#include <stdint.h>

typedef struct {
    multiboot_info_t *multiboot_addr;
    datetime_t datetime;
    uint8_t cpuid_supported;
    char cpu_manufacturer[13];
    char cpu_full_name[49];
    char *kernel_name;
    char *kernel_version;
    char *kernel_codename;
    drive_entry_t *drives;
    uint16_t drives_count;
    uint16_t *pci_devs;
    uint16_t pci_dev_count;
} kernel_globals;
