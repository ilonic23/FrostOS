#ifndef SNOW_FS_H
#define SNOW_FS_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    unsigned int year : 12;
    unsigned int month : 4;
    unsigned int day : 5;
    unsigned int hours : 5;
    unsigned int minutes : 6;
} snowfs_time_info_t;

typedef struct __attribute__((packed)) {
    uint8_t flags;
    uint32_t sector_offset; // Up to 2 TB offset (offset is in sectors)
    uint32_t size_in_sects; // up to 2 TB size
    uint16_t size_in_bytes; // To get size in bytes we add this to sector amount
                            // (max value of this should be 511)
    snowfs_time_info_t created_timestamp;
    snowfs_time_info_t modified_timestamp;
    uint8_t file_name[8];
    uint8_t file_ext[3];
    uint8_t reserved[2];
} snowfs_file_entry_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t volume_label[16];
    uint16_t sectors_for_files;
} snowfs_superblock_t;

#endif
