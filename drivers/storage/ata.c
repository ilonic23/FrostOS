#include "ata.h"
#include "../../cpu/ports.h"
#include "../../kernel/debug.h"

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

void ata_400ns_delay(uint16_t io_base, uint16_t ct_base) {
    for (int i = 0; i < 15; ++i) {
        if (i % 2 == 0) {
            (volatile void)port_byte_in(io_base + STATUS_REG);
        } else {
            (volatile void)port_byte_in(ct_base + ALT_STATUS_REG);
        }
    }
}

int ata_wait_bsy(uint16_t io_base) {
    uint8_t status;
    for (int i = 0; i < ATTEMPTS; ++i) {
        status = port_byte_in(io_base + STATUS_REG);
        if (!(status & STATUS_BSY))
            return 1;
    }
    ata_trace("BSY Timeout");
    return 0;
}

int ata_wait_drq(ata_drive_t *drive) {
    uint8_t status;
    for (int i = 0; i < ATTEMPTS; ++i) {
        status = port_byte_in(drive->io_base + STATUS_REG);
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

int ata_software_reset(uint16_t io_base, uint16_t ct_base) {
    port_byte_out(ct_base + DEV_CT_REG, 0x04);
    port_byte_out(ct_base + DEV_CT_REG, 0x00); // Clear SRST

    return ata_wait_bsy(io_base);
}

void ata_select_drive(ata_drive_t *drive) {
    uint8_t drv = drive->flags & (1u << 0) ? SLAVE : MASTER;
    port_byte_out(drive->io_base + DRV_HEAD_REG, drv);
    ata_400ns_delay(drive->io_base, drive->ct_base);
}

int ata_identify(uint8_t secondary, uint8_t slave, ata_drive_t *drive) {
    uint16_t io_base = secondary ? SECONDARY_IO_BASE : PRIMARY_IO_BASE;
    uint16_t ct_base = secondary ? SECONDARY_CT_BASE : PRIMARY_CT_BASE;
    uint8_t drv = slave ? SLAVE : MASTER;

    if (!ata_software_reset(io_base, ct_base))
        return -1;

    // Select drive
    port_byte_out(io_base + DRV_HEAD_REG, drv);
    ata_400ns_delay(io_base, ct_base);

    // Set count to 0
    port_byte_out(io_base + SECT_CNT_REG, 0);
    // Set LBAlo, LBAmid, LBAhi to 0
    port_byte_out(io_base + LBA_LO_REG, 0);
    port_byte_out(io_base + LBA_MID_REG, 0);
    port_byte_out(io_base + LBA_HI_REG, 0);

    // IDENTIFY
    port_byte_out(io_base + CMD_REG, IDENTIFY);

    uint8_t status = port_byte_in(io_base + STATUS_REG);
    if (status == 0) {
        return 1; // No drive
    }

    uint8_t is_ata = 1;

    uint32_t i;
    // BSY
    for (i = 0; i < ATTEMPTS &&
                (status = port_byte_in(io_base + STATUS_REG) & STATUS_BSY);
         ++i) {
        uint8_t lba_mid = port_byte_in(io_base + LBA_MID_REG);
        uint8_t lba_hi = port_byte_in(io_base + LBA_HI_REG);

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

    uint8_t atapi, sata;
    atapi = sata = 0;

    if (!is_ata) {
        uint8_t lba_mid = port_byte_in(io_base + LBA_MID_REG);
        uint8_t lba_hi = port_byte_in(io_base + LBA_HI_REG);

        // ATAPI
        if (lba_mid == 0x14 && lba_hi == 0xEB)
            atapi = 1;

        // SATA
        if (lba_mid == 0x3C && lba_hi == 0xC3)
            sata = 1;
    }

    // DRQ / ERR
    for (i = 0; i < ATTEMPTS; ++i) {
        status = port_byte_in(io_base + STATUS_REG);
        if (status & STATUS_DRQ)
            break;
        if (status & STATUS_ERR) {
            if (port_byte_in(io_base + ERR_REG) & ERR_ABRT) { // ATAPI
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
        drive->identify_vals[i] = port_word_in(io_base + DATA_REG);
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
void ata_str_to_c(char *dest, const uint16_t *src, int words) {
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
uint32_t ata_get_lba28_sects(ata_drive_t *drive) {
    uint32_t result =
        (uint32_t)drive->identify_vals[61] << 16 | drive->identify_vals[60];
    return result;
}

// If 0, LBA48 isn't supported
uint64_t ata_get_lba48_sects(ata_drive_t *drive) {
    if (drive->identify_vals[83] & (1u << 10)) {
        uint64_t result = (uint64_t)drive->identify_vals[103] << 48 |
                          (uint64_t)drive->identify_vals[102] << 32 |
                          (uint64_t)drive->identify_vals[101] << 16 |
                          drive->identify_vals[100];
    }
    return 0;
}

int ata_lba28_read(ata_drive_t *drive, uint32_t lba, uint8_t count,
                   uint16_t *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    uint32_t total_sects = ata_get_lba28_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to read more than available.");
        return 0;
    }

    uint8_t slave = (drive->flags & 1u) ? (1u << 4) : 0;
    port_byte_out(drive->io_base + DRV_HEAD_REG,
                  0xE0 | slave | ((lba >> 24) & 0x0F));

    port_byte_out(drive->io_base + SECT_CNT_REG, count);

    port_byte_out(drive->io_base + LBA_LO_REG, (lba >> 0) & 0xFF);
    port_byte_out(drive->io_base + LBA_MID_REG, (lba >> 8) & 0xFF);
    port_byte_out(drive->io_base + LBA_HI_REG, (lba >> 16) & 0xFF);

    port_byte_out(drive->io_base + CMD_REG, READ_SECTS);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            *buffer++ = port_word_in(drive->io_base + DATA_REG);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }

    return 1;
}

int ata_cache_flush(uint16_t io_base) {
    port_byte_out(io_base + CMD_REG, CACHE_FLUSH);
    return ata_wait_bsy(io_base);
}

int ata_lba28_write(ata_drive_t *drive, uint32_t lba, uint8_t count,
                    uint16_t *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    uint32_t total_sects = ata_get_lba28_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to write more than available.");
        return 0;
    }

    uint8_t slave = (drive->flags & 1u) ? (1u << 4) : 0;
    port_byte_out(drive->io_base + DRV_HEAD_REG,
                  0xE0 | slave | ((lba >> 24) & 0x0F));

    port_byte_out(drive->io_base + SECT_CNT_REG, count);

    port_byte_out(drive->io_base + LBA_LO_REG, (lba >> 0) & 0xFF);
    port_byte_out(drive->io_base + LBA_MID_REG, (lba >> 8) & 0xFF);
    port_byte_out(drive->io_base + LBA_HI_REG, (lba >> 16) & 0xFF);

    port_byte_out(drive->io_base + CMD_REG, WRITE_SECTS);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            port_word_out(drive->io_base + DATA_REG, *buffer++);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }
    return ata_cache_flush(drive->io_base);
}

int ata_lba48_read(ata_drive_t *drive, uint64_t lba, uint16_t count,
                   uint16_t *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    uint64_t total_sects = ata_get_lba48_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to read more than available.");
        return 0;
    }

    uint8_t slave = (drive->flags & 1u) ? (1u << 4) : 0;
    port_byte_out(drive->io_base + DRV_HEAD_REG,
                  0xE0 | slave | ((lba >> 24) & 0x0F));

    port_byte_out(drive->io_base + SECT_CNT_REG, count >> 8);

    uint8_t *lba_bytes = (uint8_t *)&lba;
    port_byte_out(drive->io_base + LBA_LO_REG, lba_bytes[3]);
    port_byte_out(drive->io_base + LBA_MID_REG, lba_bytes[4]);
    port_byte_out(drive->io_base + LBA_HI_REG, lba_bytes[5]);

    port_byte_out(drive->io_base + SECT_CNT_REG, count & 0x00FF);

    port_byte_out(drive->io_base + LBA_LO_REG, lba_bytes[0]);
    port_byte_out(drive->io_base + LBA_MID_REG, lba_bytes[1]);
    port_byte_out(drive->io_base + LBA_HI_REG, lba_bytes[2]);

    port_byte_out(drive->io_base + CMD_REG, READ_SECTS_EX);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            *buffer++ = port_word_in(drive->io_base + DATA_REG);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }
    return 1;
}

int ata_lba48_write(ata_drive_t *drive, uint64_t lba, uint16_t count,
                    uint16_t *buffer) {
    if (!buffer) {
        ata_trace("Buffer is null.");
        return 0;
    }
    if (!count) {
        ata_trace("Sector count is 0.");
        return 0;
    }
    uint64_t total_sects = ata_get_lba48_sects(drive);
    if (lba >= total_sects) {
        ata_trace("LBA is more than total sectors.");
        return 0;
    }
    if ((lba + count) > total_sects) {
        ata_trace("Attempt to read more than available.");
        return 0;
    }

    uint8_t slave = (drive->flags & 1u) ? (1u << 4) : 0;
    port_byte_out(drive->io_base + DRV_HEAD_REG,
                  0xE0 | slave | ((lba >> 24) & 0x0F));

    port_byte_out(drive->io_base + SECT_CNT_REG, count >> 8);

    uint8_t *lba_bytes = (uint8_t *)&lba;
    port_byte_out(drive->io_base + LBA_LO_REG, lba_bytes[3]);
    port_byte_out(drive->io_base + LBA_MID_REG, lba_bytes[4]);
    port_byte_out(drive->io_base + LBA_HI_REG, lba_bytes[5]);

    port_byte_out(drive->io_base + SECT_CNT_REG, count & 0x00FF);

    port_byte_out(drive->io_base + LBA_LO_REG, lba_bytes[0]);
    port_byte_out(drive->io_base + LBA_MID_REG, lba_bytes[1]);
    port_byte_out(drive->io_base + LBA_HI_REG, lba_bytes[2]);

    port_byte_out(drive->io_base + CMD_REG, READ_SECTS_EX);
    ata_400ns_delay(drive->io_base, drive->ct_base);

    for (int i = 0; i < count; ++i) {
        if (!ata_wait_drq(drive))
            return 0;

        for (int j = 0; j < 256; ++j)
            port_word_out(drive->io_base + DATA_REG, *buffer++);
        ata_400ns_delay(drive->io_base, drive->ct_base);
    }
    return ata_cache_flush(drive->io_base);
}

uint8_t ata_perform_device_diagnostics(ata_drive_t *drive) {
    ata_select_drive(drive);
    port_byte_out(drive->io_base + CMD_REG, DEVICE_DIAG);
    while (port_byte_in(drive->io_base + STATUS_REG) & (1u << 7))
        asm volatile("hlt");
    return port_byte_in(drive->io_base + ERR_REG);
}
