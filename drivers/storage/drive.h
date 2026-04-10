#ifndef DRIVE_H
#define DRIVE_H

#include "ahci.h"
#include "ata.h"
#include <stdint.h>

typedef enum {
    DRIVE_NULL = 0,
    DRIVE_AHCI_DEV_NULL,
    DRIVE_AHCI_DEV_SATA,
    DRIVE_AHCI_DEV_SEMB,
    DRIVE_AHCI_DEV_PM,
    DRIVE_AHCI_DEV_SATAPI,
    DRIVE_ATA,
    DRIVE_ATA_ATAPI,
    DRIVE_ATA_SATA
} drive_type_t;

typedef struct {
    drive_type_t type;
    union {
        HBA_PORT *ahci_hba_port;
        ata_drive_t *ata_drive;
    } control;
} drive_entry_t;

drive_entry_t drive_ata_init(ata_drive_t *drive);
int drive_lba28_read(drive_entry_t *drive, uint32_t lba, uint8_t count,
                     uint16_t *buffer);
int drive_lba28_write(drive_entry_t *drive, uint32_t lba, uint8_t count,
                      uint16_t *buffer);
uint32_t drive_lba28_sects(drive_entry_t *drive);

#endif
