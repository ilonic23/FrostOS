#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>
#include <stddef.h>

// https://wiki.osdev.org/RSDP
typedef struct __attribute__ ((packed)) {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;      // deprecated since version 2.0

    // XSDP new fields
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} rsdp_t;

// https://wiki.osdev.org/RSDT
typedef struct __attribute__ ((packed)) {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} sdt_header_t;

typedef struct __attribute__ ((packed)) {
    sdt_header_t header;
    uint32_t *other_sdt_ptr;
} rsdt_t;

typedef struct __attribute__ ((packed)) {
    sdt_header_t header;
    uint64_t *other_sdt_ptr;
} xsdt_t;

rsdp_t *find_rsdp();
uint8_t rsdp_checksum(rsdp_t *ptr, size_t size);
uint8_t validate_rsdp(rsdp_t *ptr);

#endif
