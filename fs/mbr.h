#ifndef MBR_H
#define MBR_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t attributes;
    uint8_t start_chs[3];
    uint8_t type;
    uint8_t last_chs[3];
    uint32_t start_lba;
    uint32_t sectors; // How much sectors in the partition
} partition_entry_t;

typedef struct __attribute__((packed)) {
    uint8_t bootstrap[440];
    uint32_t disk_signature;
    uint16_t reserved;
    partition_entry_t partitions[4];
    uint16_t signature; // 0xAA55
} mbr_t;

#endif
