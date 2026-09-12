#include "kernel.h"
#include "../cpu/isr.h"
#include "../drivers/clock.h"
#include "../drivers/cpuid.h"
#include "../drivers/display/display.h"
#include "../drivers/display/font8x8.h"
#include "../drivers/pci.h"
#include "../drivers/storage/ata.h"
#include "../libc/ctype.h"
#include "../libc/mem.h"
#include "../libc/stdio.h"
#include "../multiboot/multiboot.h"
#include "debug.h"
#include "globals.h"
#include "log.h"
#include "shell.h"

#include <stdint.h>

extern uint8_t kernel_start, kernel_end;

static kernel_globals globals;
static ata_drive_t *ata_drives;
static vga_info_t vga;

kernel_globals *get_kernel_globals() { return &globals; }

void pci_scan_devs() {
    for (uint16_t bus = 0; bus < 256; ++bus)
        for (uint8_t slot = 0; slot < 32; ++slot)
            if (pci_get_vendor(bus, slot) != 0xFFFF) {
                globals.pci_devs[globals.pci_dev_count++] = (bus << 8) | slot;
            }
}

struct stack_frame {
    struct stack_frame *ebp;
    uint32_t ret_addr;
};

void kernel_panic(uint32_t isr_num, char *exception_message,
                  registers_t *regs) {
    struct stack_frame *frame = (struct stack_frame *)(uintptr_t)regs->ebp;

    trace("Kernel panic: Interrupt number: 0x%X Error code: 0x%X Exception "
          "message: %s",
          isr_num, regs->err_code, exception_message);
    trace("Registers dump: EAX: 0x%X | EBX: 0x%X | ECX: 0x%X | EDX: 0x%X",
          regs->eax, regs->ebx, regs->ecx, regs->edx);
    trace("EDI: 0x%X | ESI: 0x%X | EBP: 0x%X", regs->edi, regs->esi, regs->ebp);
    trace("EFLAGS: 0x%X", regs->eflags);
    display_set_background(DISPLAY_COLOR(100, 20, 80));
    display_clear_screen();
    printf("Kernel Panic - CPU exception\n");
    printf("CPU regs\n");
    printf("EAX: 0x%X EBX: 0x%X ECX: 0x%X EDX: 0x%X\n", regs->eax, regs->ebx,
           regs->ecx, regs->edx);
    printf("EDI: 0x%X ESI: 0x%X EBP: 0x%X\n", regs->edi, regs->esi, regs->ebp);
    printf("EFLAGS: 0x%X\n", regs->eflags);
    printf("Interrupt: 0x%x\nError code: 0x%X\n", isr_num, regs->err_code);
    printf("Message: %s\n", exception_message);
    printf("Trace:\n");
#define MAX_FRAMES 8
    for (uint32_t i = 0; i < MAX_FRAMES && frame && frame->ret_addr; ++i) {
        trace("#%u: 0x%X", i, frame->ret_addr);
        printf("#%u: 0x%X\n", i, frame->ret_addr);
        if ((uint32_t)(uintptr_t)frame->ebp <= (uint32_t)(uintptr_t)frame)
            break;
        frame = frame->ebp;
    }
    printf("\nSystem halted.");
    for (;;)
        asm volatile("hlt");
}

// Function that is called at the very start of the kernel to perform needed
// operations before, like initializing everything
void kstart(uint32_t addr) {
    trace("Init ISR");
    isr_install();

    globals.multiboot_addr = (multiboot_info_t *)(uintptr_t)addr;
    display_init_fb((multiboot_info_t *)(uintptr_t)addr);

    display_font font = (display_font){.glyphs = (uint8_t *)font_8x8_glyphs,
                                       .glyph_count = 128, // 128
                                       .glyph_width = 8,
                                       .glyph_height = 8};
    display_set_font(&font);
    display_set_background(DISPLAY_COLOR(0, 0, 0));
    display_set_foreground(DISPLAY_COLOR(255, 255, 255));
    display_clear_screen();

    trace("Init IRQ");
    sys_log_msg("Initializing IRQ...");
    irq_install();

    trace("Read RTC");
    sys_log_msg("Reading time...");
    rtc_read_datetime(&globals.datetime);

    trace("Fetch CPU");
    sys_log_msg("Fetching CPU...");
    if (check_cpuid()) {
        globals.cpuid_supported = 1;
        get_cpu_manufacturer(globals.cpu_manufacturer);
        get_cpu_human_id(globals.cpu_full_name);
    } else {
        globals.cpuid_supported = 0;
        memset((uint8_t *)globals.cpu_manufacturer, 0, 13);
        memset((uint8_t *)globals.cpu_full_name, 0, 49);
    }

    trace("Init kernel heap");
    sys_log_msg("Setting up kernel heap...");
    multiboot_info_t *mbi = globals.multiboot_addr;
    uint32_t longest_size = 0;
    uint32_t free_address = 0x000000;
    if (mbi->flags & (1u << 6)) {
        for (uint32_t i = 0; i < mbi->mmap_length;
             i += sizeof(multiboot_mmap_entry_t)) {
            multiboot_mmap_entry_t *entry =
                (multiboot_mmap_entry_t *)(uintptr_t)(mbi->mmap_addr + i);

            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE &&
                (entry->addr >> 32) == 0 && entry->len > longest_size) {
                free_address = entry->addr;
                longest_size = entry->len;
            }
        }
    } else {
        sys_log_err(
            "Multiboot memory maps unavailable, defaulting to 0x400000.");
        longest_size = 0x800000;
    }
    if (&kernel_start == (uint8_t *)(uintptr_t)free_address)
        free_address = (uintptr_t)&kernel_end;
    kmalloc_init((void *)(uintptr_t)(free_address + 0x400000),
                 longest_size - 0x400000);

    globals.pci_devs = kcalloc(256, sizeof(uint16_t));
    globals.pci_dev_count = 0;
    trace("Scan PCI");
    sys_log_msg("Scanning PCI...");
    pci_scan_devs();

    globals.drives = kcalloc(4, sizeof(ata_drive_t));
    globals.drives_count = 0;
    ata_drives = kcalloc(4, sizeof(ata_drive_t));
    trace("Scan ATA");
    sys_log_msg("Scanning ATA drives...");
    for (int i = 0; i < 4; ++i) {
        int bus = i / 2;
        int drive = i % 2;
        int result = ata_identify(bus, drive, &ata_drives[i]);
        if (result != -1 && result != 1 && result != 2)
            globals.drives[globals.drives_count++] = ata_drives[i];
    }

    globals.kernel_name = "FrostOS";
    globals.kernel_version = "0.2A";
    globals.kernel_codename = "Ice Palaces";

    if (!globals.cpuid_supported)
        sys_log_err("CPUID instruction not supported.");
    sys_log_msg("Done setting up.");
}

void kernel_main(uint32_t magic, uint32_t addr) {
    trace("Bootloader magic: 0x%X", magic);
    trace("Bootloader flags: 0x%X\n", globals.multiboot_addr->flags);
    kstart(addr);
    display_clear_screen();
    printf("%s %s\n", globals.kernel_name, globals.kernel_version);

    shell();
}
