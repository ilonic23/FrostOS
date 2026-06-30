#ifndef MBR_H
#define MBR_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t boot_indicator;
    uint8_t start_head;
    uint16_t start_sector : 6;
    uint16_t start_cylinder : 10;
    uint8_t system_id;
    uint8_t end_head;
    uint16_t end_sector : 6;
    uint16_t end_cylinder : 10;
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
