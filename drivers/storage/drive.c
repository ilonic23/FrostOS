#include "drive.h"

drive_entry_t drive_ata_init(ata_drive_t *drive) {
    uint8_t ata = drive->flags & (1u << 1);
    uint8_t atapi = drive->flags & (1u << 2);
    uint8_t sata = drive->flags & (1u << 3);
    drive_type_t type;
    if (ata)
        type = DRIVE_ATA;
    else if (atapi)
        type = DRIVE_ATA_ATAPI;
    else if (sata)
        type = DRIVE_ATA_SATA;
    else
        type = DRIVE_NULL;
    drive_entry_t entry = {.type = type, .control.ata_drive = drive};
    return entry;
}

int drive_lba28_read(drive_entry_t *drive, uint32_t lba, uint8_t count,
                     uint16_t *buffer) {
    switch (drive->type) {
    case DRIVE_ATA:
    case DRIVE_ATA_ATAPI:
    case DRIVE_ATA_SATA:
        return ata_lba28_read(drive->control.ata_drive, lba, count, buffer);
    default:
        return 0;
    }
}

int drive_lba28_write(drive_entry_t *drive, uint32_t lba, uint8_t count,
                      uint16_t *buffer) {
    switch (drive->type) {
    case DRIVE_ATA:
    case DRIVE_ATA_ATAPI:
    case DRIVE_ATA_SATA:
        return ata_lba28_write(drive->control.ata_drive, lba, count, buffer);
    default:
        return 0;
    }
}
