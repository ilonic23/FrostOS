#include "ext2.h"
#include "../drivers/storage/drive.h"
#include "../libc/mem.h"

ext2_fs_entry *ext2_init(drive_entry_t *d) {
    uint8_t status = 1;
    ext2_fs_entry *entry = kmalloc(sizeof(ext2_fs_entry));
    if (!d || !entry) {
        status = 0;
        goto cleanup;
    }
    entry->drive = d;
    ext2_full_superblock_t *superblock_sector = kcalloc(512, sizeof(uint16_t));
    if (!superblock_sector) {
        status = 0;
        goto cleanup;
    }
    if (!drive_lba28_read(d, 2, 2, (uint16_t *)superblock_sector)) {
        status = 0;
        goto cleanup;
    }
    entry->superblock = &superblock_sector->base;
    if (entry->superblock->signature != 0xEF53) {
        status = 0;
        goto cleanup;
    }
    entry->superblock_ex = &superblock_sector->extended;

cleanup:
    if (!status) {
        if (entry)
            kfree(entry);
        if (superblock_sector)
            kfree(superblock_sector);
        return NULL;
    }
    return entry;
}

void ext2_free(ext2_fs_entry *entry) {
    if (!entry)
        return;
    if (!entry->drive)
        return;

    kfree(entry->superblock);
}

uint32_t ext2_get_block_size(ext2_fs_entry *entry) {
    if (!entry)
        return 0;
    return 1024 << entry->superblock->block_size;
}

int ext2_read_block(ext2_fs_entry *entry, uint32_t block, uint8_t *buffer) {
    if (!entry)
        return 0;
    uint32_t sectors = ext2_get_block_size(entry) / 512;
    return drive_lba28_read(entry->drive, block * sectors, sectors,
                            (uint16_t *)buffer);
}

uint32_t ext2_bgdt_block(ext2_fs_entry *entry) {
    return ((1024 << entry->superblock->block_size) == 1024) ? 2 : 1;
}

int ext2_read_inode(ext2_fs_entry *entry, uint32_t num, ext2_inode_t *inode) {
    uint32_t block_size = ext2_get_block_size(entry);
    uint32_t group = (num - 1) / entry->superblock->inodes_in_group;
    uint32_t pos_in_group = (num - 1) % entry->superblock->inodes_in_group;

    uint16_t *bgdt = kmalloc(block_size);
    if (!bgdt)
        return 0;
    if (!ext2_read_block(entry, ext2_bgdt_block(entry), (uint8_t *)bgdt)) {
        kfree(bgdt);
        return 0;
    }

    uint32_t inode_table_block =
        ((ext2_block_group_descriptor_t *)bgdt)[group].inode_table_start_block;
    uint32_t inode_offset = (inode_table_block * block_size) +
                            (pos_in_group * entry->superblock_ex->inode_size);

    uint8_t *inode_val = kmalloc(block_size);
    if (!drive_lba28_read(entry->drive, inode_offset / 512, block_size / 512,
                          (uint16_t *)inode_val)) {
        kfree(bgdt);
        kfree(inode_val);
        return 0;
    }

    memcpy(inode, (ext2_inode_t *)(inode_val + inode_offset % 512),
           sizeof(ext2_inode_t));
    return 1;
}

int ext2_read_inode_block(ext2_fs_entry *entry, ext2_inode_t *inode,
                          uint32_t block, uint8_t *buffer) {
    if (block < 12)
        return ext2_read_block(entry, inode->direct_block_ptrs[block], buffer);
    block -= 12;

    uint32_t block_size = ext2_get_block_size(entry);
    uint32_t P = block_size / sizeof(uint32_t);
    uint32_t P2 = P * P;
    uint32_t P3 = P * P * P;

    uint8_t *buf = kmalloc(block_size);
    uint8_t *buf2 = kmalloc(block_size);
    if (!buf || !buf2) {
        kfree(buf);
        kfree(buf2);
        return 0;
    }

    int result = 0;

    if (block < P) {
        if (ext2_read_block(entry, inode->indirect_block_ptr, buf))
            result = ext2_read_block(entry, ((uint32_t *)buf)[block], buffer);
        goto cleanup;
    }
    block -= P;

    if (block < P2) {
        if (ext2_read_block(entry, inode->doubly_indirect_block_ptr, buf))
            if (ext2_read_block(entry, ((uint32_t *)buf)[block / P], buf2))
                result = ext2_read_block(entry, ((uint32_t *)buf2)[block % P],
                                         buffer);
        goto cleanup;
    }
    block -= P2;

    if (block < P3) {
        if (ext2_read_block(entry, inode->triply_indirect_block_ptr, buf))
            if (ext2_read_block(entry, ((uint32_t *)buf)[block / P2], buf2))
                if (ext2_read_block(entry, ((uint32_t *)buf2)[(block % P2) / P],
                                    buf2))
                    result = ext2_read_block(
                        entry, ((uint32_t *)buf2)[block % P], buffer);
        goto cleanup;
    }

cleanup:
    kfree(buf);
    kfree(buf2);
    return result;
}

int ext2_read_inode_whole(ext2_fs_entry *entry, ext2_inode_t *inode,
                          uint8_t *buffer) {
    uint32_t block_size = ext2_get_block_size(entry);
    uint32_t block_count = (inode->size_low + block_size - 1) / block_size;

    for (uint32_t i = 0; i < block_count; ++i) {
        if (!ext2_read_inode_block(entry, inode, i, buffer))
            return 0;
        buffer += block_size;
    }
    return 1;
}
