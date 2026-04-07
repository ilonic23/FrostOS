#ifndef ATA_H
#define ATA_H

#include <stdint.h>

int ata_identify();
uint32_t ata_get_lba28_sects();
uint64_t ata_get_lba48_sects();
int ata_lba28_read(uint32_t lba, uint8_t count, uint16_t *buffer);
int ata_lba28_write(uint32_t lba, uint8_t count, uint16_t *buffer);
#endif
