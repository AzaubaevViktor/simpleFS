#pragma once

#include <cstdint>

#define INODE_SIZE (128)
#define BS (1024)
#define INODES_IN_BLOCK  (BS / INODE_SIZE)
#define ROOT_DATABLOCK_N (1)

#define FS_NAME_LEN (128)

#pragma pack(push)
#pragma pack(1)

#define elif else if

typedef struct _Block {
	uint8_t data[BS];
} Block;

#define SUPERBLOCK_TRASH_SIZE (BS - FS_NAME_LEN - 4 * 9)

typedef struct _Superblock {
	uint32_t block_count;

	// номера
	uint32_t bit_mask_inode;
	uint32_t first_inode;
	uint32_t bit_mask_data;
	uint32_t start_data;

	uint32_t inodes_count;
	uint32_t data_blocks_count;

	uint32_t free_inodes;
	uint32_t free_data_blocks;

	uint8_t fs_name[FS_NAME_LEN];

	uint8_t trash[SUPERBLOCK_TRASH_SIZE];
} Superblock;

#define INODE_DATA_BLOCKS ((INODE_SIZE - 3 * 8 - 4) / 4)
typedef struct _INode {
	uint64_t ctime;
	uint64_t mtime;
	uint64_t atime;

	uint32_t data_blocks_numbers[INODE_DATA_BLOCKS];
	uint32_t next_inode;
} INode;

#define OBJ_NAME_LENGTH (BS-8-4-4-1)
#define FS_CATALOG (1)
#define FS_FILE (2)

typedef struct _MetaInf {
	uint64_t length;
	uint32_t number;
	uint32_t inode_number;
	uint8_t obj_type;
	uint8_t name[OBJ_NAME_LENGTH];
} MetaInf;

#pragma pack(pop)


