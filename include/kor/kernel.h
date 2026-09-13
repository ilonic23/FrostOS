#ifndef KOR_KERNEL_H
#define KOR_KERNEL_H

#include <kor/boot/multiboot.h>
#include <kor/ll/isr.h>
#include <kor/storage/ata.h>
#include <kor/types.h>

_Noreturn void kernel_panic(u32 isr_num, i8 *exception_message,
                            registers_t *regs);

typedef struct {
    multiboot_info_t *multiboot_addr;
    u8 cpuid_supported;
    char cpu_manufacturer[13];
    char cpu_full_name[49];
    char *kernel_name;
    char *kernel_version;
    char *kernel_codename;
    ata_drive_t *drives;
    u16 drives_count;
    u16 *pci_devs;
    u16 pci_dev_count;
} kernel_globals;

#endif // KOR_KERNEL_H
