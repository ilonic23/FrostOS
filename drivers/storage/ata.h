#ifndef ATA_H
#define ATA_H

#include <stdint.h>

typedef struct {
    uint16_t io_base;
    uint16_t ct_base;
    uint8_t flags; // 0 - slave, 1 - ata, 2 - atapi, 3 - sata
    uint16_t identify_vals[256];
} ata_drive_t;

int ata_identify(uint8_t secondary, uint8_t slave, ata_drive_t *drive);
uint32_t ata_get_lba28_sects(ata_drive_t *drive);
uint64_t ata_get_lba48_sects(ata_drive_t *drive);
int ata_lba28_read(ata_drive_t *drive, uint32_t lba, uint8_t count,
                   uint16_t *buffer);
int ata_lba28_write(ata_drive_t *drive, uint32_t lba, uint8_t count,
                    uint16_t *buffer);
#endif
