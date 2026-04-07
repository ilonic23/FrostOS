#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t magic; // 'FRFS'
    uint32_t version;
    uint64_t total_blocks;
    uint64_t free_blocks;
    uint8_t uuid[16];
    uint8_t label[32];
    uint32_t flags;
    uint64_t inode_table_lba;
    uint64_t data_lba;
    uint64_t free_inodes;
    uint64_t inode_count;
} frostfs_superblock_t;

typedef struct __attribute__((packed)) {
    uint16_t mode; // 0-8 - permissions, 9-10 - type
    uint16_t uid;
    uint16_t gid;
    uint64_t size;  // 0-8 - bytes, 9-63 - sectors
    uint64_t ctime; // created time in ns from unix epoch
    uint64_t mtime; // modified time in ns from unix epoch
    uint64_t atime; // accessed time in ns from unix epoch
    uint16_t flags;
    uint64_t start_block;
} frostfs_inode_t;
