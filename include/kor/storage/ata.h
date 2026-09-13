#ifndef KOR_STORAGE_ATA_H
#define KOR_STORAGE_ATA_H

#include <kor/types.h>

typedef struct {
    u16 io_base;
    u16 ct_base;
    u8 flags; // 0 - slave, 1 - ata, 2 - atapi, 3 - sata
    u16 identify_vals[256];
} ata_drive_t;

int ata_identify(u8 secondary, u8 slave, ata_drive_t *drive);
void ata_stro_c(char *dest, const u16 *src, int words);
u32 ata_get_lba28_sects(ata_drive_t *drive);
u64 ata_get_lba48_sects(ata_drive_t *drive);
int ata_lba28_read(ata_drive_t *drive, u32 lba, u8 count, u16 *buffer);
int ata_lba28_write(ata_drive_t *drive, u32 lba, u8 count, u16 *buffer);
int ata_lba48_read(ata_drive_t *drive, u64 lba, u16 count, u16 *buffer);
int ata_lba48_write(ata_drive_t *drive, u64 lba, u16 count, u16 *buffer);
u8 ata_perform_device_diagnostics(ata_drive_t *drive);

#endif // KOR_STORAGE_ATA_H
