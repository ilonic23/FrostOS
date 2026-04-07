#ifndef ATA_H
#define ATA_H

#include <stdint.h>

int ata_identify();
uint32_t ata_get_lba28_sects();
uint64_t ata_get_lba48_sects();
#endif
