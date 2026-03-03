#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// Reads a sector (512 bytes) from disk. Parameters: lba - sector index, buffer - buffer to read to.
void ata_read_sector(uint32_t lba, char *buffer);
// Writes a sector (512 bytes) from disk. Parameters: lba - sector index, buffer - buffer to write.
void ata_write_sector(uint32_t lba, char *buffer);
uint32_t ata_get_sectors();

#endif
