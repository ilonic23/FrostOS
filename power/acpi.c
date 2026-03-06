#include "acpi.h"
#include <stdint.h>
#include <stddef.h>
#include "../libc/mem.h"

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
