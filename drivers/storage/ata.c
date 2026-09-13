#include <kor/debug.h>
#include <kor/ll/port.h>
#include <kor/storage/ata.h>

#define ata_trace(fmt, ...) trace("[ATA] " fmt, ##__VA_ARGS__)

#define MASTER 0xA0
#define SLAVE 0xB0

#define PRIMARY_IO_BASE 0x1F0
#define PRIMARY_CT_BASE 0x3F6
#define SECONDARY_IO_BASE 0x170
#define SECONDARY_CT_BASE 0x376

#define DATA_REG 0x0
#define ERR_REG 0x1
#define FEATURES_REG 0x1
#define SECT_CNT_REG 0x2
#define LBA_LO_REG 0x3
#define LBA_MID_REG 0x4
#define LBA_HI_REG 0x5
#define DRV_HEAD_REG 0x6
#define STATUS_REG 0x7
#define CMD_REG 0x7

#define ALT_STATUS_REG 0x0
#define DEV_CT_REG 0x0
#define DRIVE_ADDR_REG 0x1

#define IDENTIFY 0xEC
#define READ_SECTS 0x20
#define READ_SECTS_EX 0x24
#define WRITE_SECTS 0x30
#define DEVICE_DIAG 0x90

#define CACHE_FLUSH 0xE7

#define ATTEMPTS 100000
#define STATUS_ERR (1u << 0)
#define STATUS_DRQ (1u << 3)
#define STATUS_DF (1u << 5)
#define STATUS_BSY (1u << 7)

#define ERR_ABRT (1u << 2)

void ata_400ns_delay(u16 io_base, u16 ct_base) {
    for (int i = 0; i < 15; ++i) {
        if (i % 2 == 0) {
            (volatile void)inb(io_base + STATUS_REG);
        } else {
            (volatile void)inb(ct_base + ALT_STATUS_REG);
        }
    }
}

int ata_wait_bsy(u16 io_base) {
    u8 status;
    for (int i = 0; i < ATTEMPTS; ++i) {
        status = inb(io_base + STATUS_REG);
        if (!(status & STATUS_BSY))
            return 1;
    }
    ata_trace("BSY Timeout");
    return 0;
}

int ata_wait_drq(ata_drive_t *drive) {
    u8 status;
    for (int i = 0; i < ATTEMPTS; ++i) {
        status = inb(drive->io_base + STATUS_REG);
        if (status & STATUS_ERR) {
            ata_trace("Received an error.");
            return 0;
        }
        if (status & STATUS_DF) {
            ata_trace("Device fault.");
            return 0;
        }
        if (status & STATUS_DRQ && !(status & STATUS_BSY))
            return 1;
    }
    ata_trace("DRQ Timeout");
    return 0;
}

int ata_software_reset(u16 io_base, u16 ct_base) {
    outb(ct_base + DEV_CT_REG, 0x04);
    outb(ct_base + DEV_CT_REG, 0x00); // Clear SRST

    return ata_wait_bsy(io_base);
}

void ata_select_drive(ata_drive_t *drive) {
    u8 drv = drive->flags & (1u << 0) ? SLAVE : MASTER;
    outb(drive->io_base + DRV_HEAD_REG, drv);
    ata_400ns_delay(drive->io_base, drive->ct_base);
}

int ata_identify(u8 secondary, u8 slave, ata_drive_t *drive) {
    u16 io_base = secondary ? SECONDARY_IO_BASE : PRIMARY_IO_BASE;
    u16 ct_base = secondary ? SECONDARY_CT_BASE : PRIMARY_CT_BASE;
    u8 drv = slave ? SLAVE : MASTER;

    if (!ata_software_reset(io_base, ct_base))
        return -1;

    // Select drive
    outb(io_base + DRV_HEAD_REG, drv);
    ata_400ns_delay(io_base, ct_base);

    // Set count to 0
    outb(io_base + SECT_CNT_REG, 0);
    // Set LBAlo, LBAmid, LBAhi to 0
    outb(io_base + LBA_LO_REG, 0);
    outb(io_base + LBA_MID_REG, 0);
    outb(io_base + LBA_HI_REG, 0);

    // IDENTIFY
    outb(io_base + CMD_REG, IDENTIFY);

    u8 status = inb(io_base + STATUS_REG);
    if (status == 0) {
        return 1; // No drive
    }

    u8 is_ata = 1;

    u32 i;
    // BSY
    for (i = 0;
         i < ATTEMPTS && (status = inb(io_base + STATUS_REG) & STATUS_BSY);
         ++i) {
        u8 lba_mid = inb(io_base + LBA_MID_REG);
        u8 lba_hi = inb(io_base + LBA_HI_REG);

        // Non-ATA drive
        if (lba_mid != 0 || lba_hi != 0) {
            is_ata = 0;
            break;
        }
    }
    if (i >= ATTEMPTS) {
        ata_trace("BSY timeout on IDENTIFY");
        return -1;
    }

    u8 atapi, sata;
    atapi = sata = 0;

    if (!is_ata) {
        u8 lba_mid = inb(io_base + LBA_MID_REG);
        u8 lba_hi = inb(io_base + LBA_HI_REG);

        // ATAPI
        if (lba_mid == 0x14 && lba_hi == 0xEB)
            atapi = 1;

        // SATA
        if (lba_mid == 0x3C && lba_hi == 0xC3)
            sata = 1;
    }

    // DRQ / ERR
    for (i = 0; i < ATTEMPTS; ++i) {
        status = inb(io_base + STATUS_REG);
        if (status & STATUS_DRQ)
            break;
        if (status & STATUS_ERR) {
            if (inb(io_base + ERR_REG) & ERR_ABRT) { // ATAPI
                // TODO: Run ATAPI IDENTIFY PACKET DEVICE
            } else
                return 2; // ERR
        }
    }
    if (i >= ATTEMPTS) {
        ata_trace("BSY timeout on IDENTIFY");
        return -1;
    }

    for (int i = 0; i < 256; ++i)
        drive->identify_vals[i] = inw(io_base + DATA_REG);
    drive->io_base = io_base;
    drive->ct_base = ct_base;
    drive->flags |= ((slave ? 1 : 0) << 0);

    if (atapi) {
        drive->flags |= ((atapi ? 1 : 0) << 2);
        return 3; // ATAPI detected, OK
    }
    if (sata) {
        drive->flags |= ((sata ? 1 : 0) << 3);
        return 4; // SATA detected, OK
    }
    drive->flags |= (1u << 1);
    return 0; // ATA detected, OK
}

// https://read.seas.harvard.edu/cs161/2019/pdf/ata-atapi-8.pdf#M6.9.67214.Head2.329.ATA.string.convention
void ata_str_to_c(char *dest, const u16 *src, int words) {
    for (int i = 0; i < words; ++i) {
        dest[i * 2] = (char)(src[i] >> 8);
        dest[i * 2 + 1] = (char)(src[i] & 0xFF);
    }
    dest[words * 2] = '\0';

    // Trim trailing spaces
    int len = words * 2;
    while (len > 0 && dest[len - 1] == ' ')
        dest[--len] = '\0';
}

// If 0, LBA28 isn't supported
u32 ata_get_lba28_sects(ata_drive_t *drive) {
    u32 result = (u32)drive->identify_vals[61] << 16 | drive->identify_vals[60];
    return result;
}

// If 0, LBA48 isn't supported
u64 ata_get_lba48_sects(ata_drive_t *drive) {
    if (drive->identify_vals[83] & (1u << 10)) {
        u64 result = (u64)drive->identify_vals[103] << 48 |
                     (u64)drive->identify_vals[102] << 32 |
                     (u64)drive->identify_vals[101] << 16 |
                     drive->identify_vals[100];
    }
    return 0;
}

int ata_lba28_read(ata_drive_t *drive, u32 lba, u8 count, u16 *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    u32 total_sects = ata_get_lba28_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to read more than available.");
        return 0;
    }

    u8 slave = (drive->flags & 1u) ? (1u << 4) : 0;
    outb(drive->io_base + DRV_HEAD_REG, 0xE0 | slave | ((lba >> 24) & 0x0F));

    outb(drive->io_base + SECT_CNT_REG, count);

    outb(drive->io_base + LBA_LO_REG, (lba >> 0) & 0xFF);
    outb(drive->io_base + LBA_MID_REG, (lba >> 8) & 0xFF);
    outb(drive->io_base + LBA_HI_REG, (lba >> 16) & 0xFF);

    outb(drive->io_base + CMD_REG, READ_SECTS);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            *buffer++ = inw(drive->io_base + DATA_REG);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }

    return 1;
}

int ata_cache_flush(u16 io_base) {
    outb(io_base + CMD_REG, CACHE_FLUSH);
    return ata_wait_bsy(io_base);
}

int ata_lba28_write(ata_drive_t *drive, u32 lba, u8 count, u16 *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    u32 total_sects = ata_get_lba28_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to write more than available.");
        return 0;
    }

    u8 slave = (drive->flags & 1u) ? (1u << 4) : 0;
    outb(drive->io_base + DRV_HEAD_REG, 0xE0 | slave | ((lba >> 24) & 0x0F));

    outb(drive->io_base + SECT_CNT_REG, count);

    outb(drive->io_base + LBA_LO_REG, (lba >> 0) & 0xFF);
    outb(drive->io_base + LBA_MID_REG, (lba >> 8) & 0xFF);
    outb(drive->io_base + LBA_HI_REG, (lba >> 16) & 0xFF);

    outb(drive->io_base + CMD_REG, WRITE_SECTS);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            outw(drive->io_base + DATA_REG, *buffer++);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }
    return ata_cache_flush(drive->io_base);
}

int ata_lba48_read(ata_drive_t *drive, u64 lba, u16 count, u16 *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    u64 total_sects = ata_get_lba48_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to read more than available.");
        return 0;
    }

    u8 slave = (drive->flags & 1u) ? (1u << 4) : 0;
    outb(drive->io_base + DRV_HEAD_REG, 0xE0 | slave | ((lba >> 24) & 0x0F));

    outb(drive->io_base + SECT_CNT_REG, count >> 8);

    u8 *lba_bytes = (u8 *)&lba;
    outb(drive->io_base + LBA_LO_REG, lba_bytes[3]);
    outb(drive->io_base + LBA_MID_REG, lba_bytes[4]);
    outb(drive->io_base + LBA_HI_REG, lba_bytes[5]);

    outb(drive->io_base + SECT_CNT_REG, count & 0x00FF);

    outb(drive->io_base + LBA_LO_REG, lba_bytes[0]);
    outb(drive->io_base + LBA_MID_REG, lba_bytes[1]);
    outb(drive->io_base + LBA_HI_REG, lba_bytes[2]);

    outb(drive->io_base + CMD_REG, READ_SECTS_EX);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            *buffer++ = inw(drive->io_base + DATA_REG);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }
    return 1;
}

int ata_lba48_write(ata_drive_t *drive, u64 lba, u16 count, u16 *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    u64 total_sects = ata_get_lba48_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to read more than available.");
        return 0;
    }

    u8 slave = (drive->flags & 1u) ? (1u << 4) : 0;
    outb(drive->io_base + DRV_HEAD_REG, 0xE0 | slave | ((lba >> 24) & 0x0F));

    outb(drive->io_base + SECT_CNT_REG, count >> 8);

    u8 *lba_bytes = (u8 *)&lba;
    outb(drive->io_base + LBA_LO_REG, lba_bytes[3]);
    outb(drive->io_base + LBA_MID_REG, lba_bytes[4]);
    outb(drive->io_base + LBA_HI_REG, lba_bytes[5]);

    outb(drive->io_base + SECT_CNT_REG, count & 0x00FF);

    outb(drive->io_base + LBA_LO_REG, lba_bytes[0]);
    outb(drive->io_base + LBA_MID_REG, lba_bytes[1]);
    outb(drive->io_base + LBA_HI_REG, lba_bytes[2]);

    outb(drive->io_base + CMD_REG, READ_SECTS_EX);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            outw(drive->io_base + DATA_REG, *buffer++);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }
    return ata_cache_flush(drive->io_base);
}

u8 ata_perform_device_diagnostics(ata_drive_t *drive) {
    ata_select_drive(drive);
    outb(drive->io_base + CMD_REG, DEVICE_DIAG);
    while (inb(drive->io_base + STATUS_REG) & (1u << 7))
        asm volatile("hlt");
    return inb(drive->io_base + ERR_REG);
}
