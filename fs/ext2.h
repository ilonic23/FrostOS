#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t su_reserved; // Number of blocks that are reserved for superuser
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t superblock_block_num; // Block number containing the superblock
    uint32_t
        block_size; // log2 (block size) - 10. (In other words, the number to
                    // shift 1,024 to the left by to obtain the block size)
    uint32_t fragment_size;   // log2 (fragment size) - 10. (In other words, the
                              // number to shift 1,024 to the left by to obtain
                              // the fragment size)
    uint32_t blocks_in_group; // Blocks in each block group
    uint32_t fragments_in_group; // Fragments in each block group
    uint32_t inodes_in_group;    // Inodes in each block group
    uint32_t last_mnt_time;      // POSIX time
    uint32_t last_wrt_time;      // POSIX time
    uint16_t mnt_time_fsck; // Number of times the volume has been mounted since
                            // its last consistency check
    uint16_t mnt_time_allowed; // Number of mounts allowed before a consistency
                               // check (fsck) must be done
    uint16_t signature;        // 0xEF53
    uint16_t state;            // 1 - clean, 2 - has errors
    uint16_t error_detect_action; // 1 - ignore error, 2 - remount as read only,
                                  // 3 - kernel panic
    uint16_t minor_version;
    uint32_t fsck_time;     // POSIX time of last fsck
    uint32_t fsck_interval; // Interval (in POSIX time) between forced
                            // consistency checks (fsck)
    uint32_t os_id; // Operating system ID from which the filesystem on this
                    // volume was created
    uint32_t major_version;
    uint16_t uid_res_block; // User ID that can use reserved blocks
    uint16_t gid_res_block; // Group ID that can use reserved blocks
} ext2_superblock_t;

typedef struct __attribute__((packed)) {
    uint32_t first_nr_inode; // First non-reserved inode in FS (default=11)
    uint16_t inode_size;     // Inode size in bytes (default=128)
    uint16_t block_group;    // Block group that this superblock is part of (if
                             // backup copy)
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t ro_features; // Features that if not supported, the volume must be
                          // mounted read-only
    uint8_t fs_id[16];
    uint8_t vol_name[16];
    uint8_t mount_path[64];
    uint32_t compression_used;    // Compression algorithms used
    uint8_t prealloc_blocks_file; // Number of blocks to preallocate per file
    uint8_t prealloc_blocks_dir;  // Number of blocks to preallocate per dir
    uint16_t unused1;
    uint8_t journal_id[16];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_inode_list_head; // Head of orphan inode list
    uint8_t unused2[872];
} ext2_superblock_ex_t;

typedef struct __attribute__((packed)) {
    uint32_t block_usage_bitmap_block; // Block address of block usage bitmap
    uint32_t inode_usage_bitmap_block; // Block address of inode usage bitmap
    uint32_t inode_table_start_block;
    uint16_t unallocated_blocks_in_group;
    uint16_t unallocated_inodes_in_group;
    uint16_t dirs_in_group;
    uint8_t unused[14];
} ext2_block_group_descriptor_t;

typedef struct __attribute__((packed)) {
    uint16_t type_perm;
    uint16_t uid;
    uint32_t size_low;
    uint32_t access_time;
    uint32_t creation_time;
    uint32_t modification_time;
    uint32_t deletion_time;
    uint16_t gid;
    uint16_t hard_links_count;
    uint32_t used_sects;
    uint32_t flags;
    uint32_t osspec1;
    uint32_t direct_block_ptrs[12];
    uint32_t indirect_block_ptr;
    uint32_t doubly_indirect_block_ptr;
    uint32_t triply_indirect_block_ptr;
    uint32_t generation_num;
    uint32_t file_acl;
    uint32_t size_hi; // or dir acl
    uint32_t block_addr_fragment;
    uint8_t osspec2[12];
} ext2_inode_t;

typedef struct __attribute__((packed)) {
    uint32_t inode;
    uint16_t total_size;
    uint8_t name_len;
    uint8_t type;
    char name[];
} ext2_dir_entry_t;

#endif
