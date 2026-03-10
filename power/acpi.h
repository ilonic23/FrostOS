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
    uint32_t other_sdt_ptr[];
} rsdt_t;

typedef struct __attribute__ ((packed)) {
    sdt_header_t header;
    uint64_t other_sdt_ptr[];
} xsdt_t;

typedef struct __attribute__((packed)) {
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;
    uint64_t Address;
} generic_address_structure;

typedef struct __attribute__((packed)) {
    sdt_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;

    // used in ACPI 1.0
    uint8_t reserved;

    uint8_t pref_pmp; // preferred power management profile
    uint16_t sci_int; // sci_interrupt
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_len;
    uint8_t pm1_control_len;
    uint8_t pm2_control_len;
    uint8_t pm_timer_len;
    uint8_t gpe0_len;
    uint8_t gpe1_len;
    uint8_t gpe1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    // reserved in ACPI 1.0, used in ACPI 2.0
    uint16_t boot_architecture_flags;

    uint8_t reserved2;
    uint32_t flags;

    generic_address_structure reset_reg;

    uint8_t reset_value;
    uint8_t reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    uint64_t x_firmware_control;
    uint64_t x_dsdt;

    generic_address_structure x_pm1a_event_block;
    generic_address_structure x_pm1b_event_block;
    generic_address_structure x_pm1a_control_block;
    generic_address_structure x_pm1b_control_block;
    generic_address_structure x_pm2_control_block;
    generic_address_structure x_pm_timer_block;
    generic_address_structure x_gpe0_block;
    generic_address_structure x_gpe1_block;
} fadt_t;

rsdp_t *find_rsdp();
uint8_t rsdp_checksum(rsdp_t *ptr, size_t size);
uint8_t validate_rsdp(rsdp_t *ptr);
fadt_t *find_fadt(rsdp_t *rsdp);
void acpi_enable(fadt_t *fadt);
int parse_slp_from_dsdt(fadt_t *fadt);
void acpi_shutdown(fadt_t *fadt);

#endif
