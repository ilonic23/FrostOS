#include "ata.h"
#include "../../cpu/ports.h"
#include "../../kernel/log.h"

#define MASTER 0xA0
#define SLAVE 0xB0

#define PRIMARY_IO_BASE 0x1F0
#define PRIMARY_CT_BASE 0x3F6

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

#define CACHE_FLUSH 0xE7

static uint16_t identify_vals[256];

void ata_400ns_delay() {
    for (int i = 0; i < 15; ++i) {
        if (i % 2 == 0) {
            (volatile void)port_byte_in(PRIMARY_IO_BASE + STATUS_REG);
        } else {
            (volatile void)port_byte_in(PRIMARY_CT_BASE + ALT_STATUS_REG);
        }
    }
}

void ata_software_reset() {
    port_byte_out(PRIMARY_CT_BASE + DEV_CT_REG, 0x04);
    port_byte_out(PRIMARY_CT_BASE + DEV_CT_REG, 0x00); // Clear SRST

    uint8_t status;
    while ((status = port_byte_in(PRIMARY_IO_BASE + STATUS_REG)) & (1u << 7))
        ;
}

int ata_identify() {
    ata_software_reset();
    // Select master drive
    port_byte_out(PRIMARY_IO_BASE + DRV_HEAD_REG, MASTER);

    ata_400ns_delay();

    // Set LBAlo, LBAmid, LBAhi to 0
    port_byte_out(PRIMARY_IO_BASE + LBA_LO_REG, 0);
    port_byte_out(PRIMARY_IO_BASE + LBA_MID_REG, 0);
    port_byte_out(PRIMARY_IO_BASE + LBA_HI_REG, 0);

    // IDENTIFY
    port_byte_out(PRIMARY_IO_BASE + CMD_REG, IDENTIFY);

    uint8_t status = port_byte_in(PRIMARY_IO_BASE + STATUS_REG);
    if (status == 0) {
        return 1; // No drive
    }

    uint8_t is_ata = 1;

    // BSY
    while ((status = port_byte_in(PRIMARY_IO_BASE + STATUS_REG)) & (1u << 7)) {
        uint8_t lba_mid = port_byte_in(PRIMARY_IO_BASE + LBA_MID_REG);
        uint8_t lba_hi = port_byte_in(PRIMARY_IO_BASE + LBA_HI_REG);

        // Non-ATA drive
        if (lba_mid != 0 || lba_hi != 0) {
            is_ata = 0;
            break;
        }
        sys_log_msg("BSY");
    }

    uint8_t atapi, sata;
    atapi = sata = 0;

    if (!is_ata) {
        uint8_t lba_mid = port_byte_in(PRIMARY_IO_BASE + LBA_MID_REG);
        uint8_t lba_hi = port_byte_in(PRIMARY_IO_BASE + LBA_HI_REG);

        // ATAPI
        if (lba_mid == 0x14 && lba_hi == 0xEB)
            atapi = 1;

        // SATA
        if (lba_mid == 0x3C && lba_hi == 0xC3)
            sata = 1;
    }

    // DRQ / ERR
    while (is_ata) {
        status = port_byte_in(PRIMARY_IO_BASE + STATUS_REG);
        if (status & (1u << 3))
            break;
        if (status & (1u << 0))
            return 2; // ERR
        sys_log_msg("BSY");
    }

    for (int i = 0; i < 256; ++i)
        identify_vals[i] = port_word_in(PRIMARY_IO_BASE + DATA_REG);

    if (atapi)
        return 3; // ATAPI detected, OK
    if (sata)
        return 4; // SATA detected, OK
    return 0;     // ATA detected, OK
}

// If 0, LBA28 isn't supported
uint32_t ata_get_lba28_sects() {
    uint32_t result = (uint32_t)identify_vals[61] << 16 | identify_vals[60];
    return result;
}

// If 0, LBA48 isn't supported
uint64_t ata_get_lba48_sects() {
    if (identify_vals[83] & (1u << 10)) {
        uint64_t result = (uint64_t)identify_vals[103] << 48 |
                          (uint64_t)identify_vals[102] << 32 |
                          (uint64_t)identify_vals[101] << 16 |
                          identify_vals[100];
    }
    return 0;
}

int ata_lba28_read(uint32_t lba, uint8_t count, uint16_t *buffer) {
    uint32_t total_sects = ata_get_lba28_sects();
    if (lba > total_sects)
        return 0;
    if ((lba + count) > total_sects)
        return 0;

    port_byte_out(PRIMARY_IO_BASE + DRV_HEAD_REG, 0xE0 | ((lba >> 24) & 0x0F));

    port_byte_out(PRIMARY_IO_BASE + SECT_CNT_REG, count);

    port_byte_out(PRIMARY_IO_BASE + LBA_LO_REG, (lba >> 0) & 0xFF);
    port_byte_out(PRIMARY_IO_BASE + LBA_MID_REG, (lba >> 8) & 0xFF);
    port_byte_out(PRIMARY_IO_BASE + LBA_HI_REG, (lba >> 16) & 0xFF);

    port_byte_out(PRIMARY_IO_BASE + CMD_REG, READ_SECTS);

    for (int i = 0; i < count; ++i) {
        uint8_t status;
        while (1) {
            status = port_byte_in(PRIMARY_IO_BASE + STATUS_REG);
            if (status & (1u << 0))
                return 0;
            if (status & (1u << 7))
                continue;
            if (status & (1u << 3))
                break;
        }

        for (int j = 0; j < 256; ++j)
            *buffer++ = port_word_in(PRIMARY_IO_BASE + DATA_REG);
        ata_400ns_delay();
    }

    return 1;
}

void ata_cache_flush() {
    port_byte_out(PRIMARY_IO_BASE + CMD_REG, CACHE_FLUSH);
    while (port_byte_in(PRIMARY_IO_BASE + STATUS_REG) & (1u << 7))
        ;
}

int ata_lba28_write(uint32_t lba, uint8_t count, uint16_t *buffer) {
    uint32_t total_sects = ata_get_lba28_sects();
    if (lba > total_sects)
        return 0;
    if ((lba + count) > total_sects)
        return 0;

    port_byte_out(PRIMARY_IO_BASE + DRV_HEAD_REG, 0xE0 | ((lba >> 24) & 0x0F));

    port_byte_out(PRIMARY_IO_BASE + SECT_CNT_REG, count);

    port_byte_out(PRIMARY_IO_BASE + LBA_LO_REG, (lba >> 0) & 0xFF);
    port_byte_out(PRIMARY_IO_BASE + LBA_MID_REG, (lba >> 8) & 0xFF);
    port_byte_out(PRIMARY_IO_BASE + LBA_HI_REG, (lba >> 16) & 0xFF);

    port_byte_out(PRIMARY_IO_BASE + CMD_REG, WRITE_SECTS);

    for (int i = 0; i < count; ++i) {
        uint8_t status;
        while (1) {
            status = port_byte_in(PRIMARY_IO_BASE + STATUS_REG);
            if (status & (1u << 0))
                return 0;
            if (status & (1u << 7))
                continue;
            if (status & (1u << 3))
                break;
        }

        for (int j = 0; j < 256; ++j)
            port_word_out(PRIMARY_IO_BASE + DATA_REG, *buffer++);
        ata_400ns_delay();
    }
    ata_cache_flush();
    return 1;
}
