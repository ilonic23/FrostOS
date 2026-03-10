#include "acpi.h"
#include <stdint.h>
#include <stddef.h>
#include "../libc/stdio.h"
#include "../cpu/ports.h"
#include "../libc/mem.h"

static uint16_t slp_typ_a = 0, slp_typ_b = 0;

rsdp_t *find_rsdp() {
    uintptr_t ebda_segment = *(uint16_t *)0x40E;
    uintptr_t ebda_address = ebda_segment << 4;

    if (ebda_address > 0x80000 && ebda_address < 0xA0000) {
        for (uintptr_t addr = ebda_address; addr < ebda_address + 1024; ++addr) {
            rsdp_t *candidate = (rsdp_t *)addr;
            if (memcmp(candidate->Signature, "RSD PTR ", 8) == 0)
                if (validate_rsdp(candidate)) return candidate;
        }
    }


    for (uintptr_t addr = 0x000E0000; addr < 0x00100000; ++addr) {
        rsdp_t *candidate = (rsdp_t *)addr;
        if (memcmp(candidate->Signature, "RSD PTR ", 8) == 0)
            if (validate_rsdp(candidate)) return candidate;
    }
    return NULL;
}

uint8_t rsdp_checksum(rsdp_t *ptr, size_t size) {
    uint8_t sum = 0;
    for (size_t i = 0; i < size; i++) sum += ((char *)ptr)[i];
    return sum == 0;
}

// 0 - invalid
// 1 - ACPI 1.0
// 2 - ACPI 2.0
// 3 - First 20 bytes valid
uint8_t validate_rsdp(rsdp_t *ptr) {
    if (rsdp_checksum(ptr, 20)) {
        if (ptr->Revision < 2) return 1;
        if (ptr->Revision >= 2) {
            if (rsdp_checksum(ptr, ptr->Length)) return 2;
            return 3;
        }
    }
    return 0;
}

uint8_t sdt_checksum(sdt_header_t *ptr) {
    uint8_t sum = 0;
    for (uint32_t i = 0; i < ptr->Length; ++i)
        sum += ((char *)ptr)[i];
    return sum == 0;
}

uint32_t rsdt_entries_len(rsdt_t *rsdt) {
    uint32_t entry_count = (rsdt->header.Length - sizeof(sdt_header_t)) / sizeof(uint32_t);
    return entry_count;
}

uint64_t xsdt_entries_len(xsdt_t *xsdt) {
    uint64_t entry_count = (xsdt->header.Length - sizeof(sdt_header_t)) / sizeof(uint64_t);
    return entry_count;
}

fadt_t *find_fadt(rsdp_t *rsdp) {
    uint8_t rsdp_ver = validate_rsdp(rsdp);
    // ACPI 1.0
    if (rsdp_ver == 1 || rsdp_ver == 3) {
        rsdt_t *rsdt = (rsdt_t *)rsdp->RsdtAddress;
        uint32_t entries = rsdt_entries_len(rsdt);

        for (uint32_t i = 0; i < entries; ++i) {
            sdt_header_t *h = (sdt_header_t *)rsdt->other_sdt_ptr[i];
            if (memcmp(h->Signature, "FACP", 4) == 0) return (fadt_t *)h;
        }
    }
    // ACPI 2.0
    if (rsdp_ver == 2) {
        xsdt_t *xsdt = (xsdt_t *)rsdp->XsdtAddress;
        uint64_t entries = xsdt_entries_len(xsdt);

        for (uint64_t i = 0; i < entries; ++i) {
            sdt_header_t *h = (sdt_header_t *)xsdt->other_sdt_ptr[i];
            if (memcmp(h->Signature, "FACP", 4) == 0) return (fadt_t *)h;
        }
    }
    return NULL;
}

void acpi_enable(fadt_t *fadt) {
    // Is ACPI is already enabled? (SCI_EN = bit 0 of PM1a_CNT)
    if (port_word_in((uint16_t)fadt->pm1a_control_block) & 1)
        return; // Already enabled

    if (fadt->smi_command_port == 0 || fadt->acpi_enable == 0)
        return; // SMI not supported

    // Send enable command
    asm volatile ("outb %0, %1" : : "a"(fadt->acpi_enable),
                                        "Nd"((uint16_t)fadt->smi_command_port));

    // Wait for SCI_EN to go high
    for (int i = 0; i < 300; i++) {
        if (port_word_in((uint16_t)fadt->pm1a_control_block) & 1)
            return;
    }
}

// TODO: Make a real AML interpreter
int parse_slp_from_dsdt(fadt_t *fadt) {
    uint8_t *dsdt = (uint8_t *)fadt->dsdt;
    sdt_header_t *dsdt_header = (sdt_header_t *)dsdt;
    if (!sdt_checksum(dsdt_header)) return 0; // DSDT checksum failed.
    uint32_t len = dsdt_header->Length;

    // Prevent access beyond DSDT
    // Search for "_S5_" AML name object
    for (uint32_t i = 0; i < len - 8; ++i) {
        // AML NameOp = 0x08, then "_S5_"
        if (dsdt[i] == 0x08)
            if (dsdt[i + 1] == '_' && dsdt[i + 2] == 'S' &&
                dsdt[i + 3] == '5' && dsdt[i + 4] == '_') {
                printf("First\n");

            // Skip to package OpCode (0x12)
            // Sometimes there's a backslash scope prefix (0x5C)
            uint8_t *p = &dsdt[i+5];
            if (*p == 0x12) p++; // Package opcode
            p++;

            // Package contents: 4 bytes, we need bytes 0 and 1
            // Each value is encoded as BytePrefix(0x0A) + value
            if (p[0] == 0x0A) {
                slp_typ_a = (uint16_t)p[1];
                if (p[2] == 0x0A)
                    slp_typ_b = (uint16_t)p[3];
                return 1;
            }

            // Or as plain zero-byte (ByteConst 0)
            slp_typ_a = p[0];
            slp_typ_b = p[2];
            return 1;
        }
        else if (dsdt[i] == 0x5C && dsdt[i+1] == 0x08) {
            if (dsdt[i + 2] == '_' && dsdt[i + 3] == 'S' &&
                dsdt[i + 4] == '5' && dsdt[i + 5] == '_') {
                printf("Second\n");

                // Skip to package OpCode (0x12)
                // Sometimes there's a backslash scope prefix (0x5C)
                uint8_t *p = &dsdt[i+5];
                if (*p == 0x12) p++; // Package opcode
                p++;

                // Package contents: 4 bytes, we need bytes 0 and 1
                // Each value is encoded as BytePrefix(0x0A) + value
                if (p[0] == 0x0A) {
                    slp_typ_a = (uint16_t)p[1];
                    if (p[2] == 0x0A)
                        slp_typ_b = (uint16_t)p[3];
                    return 1;
                }

                // Or as plain zero-byte (ByteConst 0)
                slp_typ_a = p[0];
                slp_typ_b = p[2];
                return 1;
            }
        }
    }
    return 0;
}


void acpi_shutdown(fadt_t *fadt) {
    uint16_t SLP_EN  = (1 << 13);
    uint16_t pm1a    = (uint16_t)fadt->pm1a_control_block;
    uint16_t pm1b    = (uint16_t)fadt->pm1b_control_block;

    // SLP_TYP occupies bits 10-12
    uint16_t cmd_a = (uint16_t)((slp_typ_a << 10) | SLP_EN);
    uint16_t cmd_b = (uint16_t)((slp_typ_b << 10) | SLP_EN);

    port_word_out(pm1a, cmd_a);
    if (pm1b != 0)
        port_word_out(pm1b, cmd_b);

    // Should never reach here
    printf("slp_typ_a: %u slp_typ_b: %u", slp_typ_a, slp_typ_b);
    for (;;) __asm__ volatile ("hlt");
}
