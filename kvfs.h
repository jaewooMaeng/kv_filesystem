#define SIMULA_FS_TYPE 0x13090D15 /* Magic Number for our file system */
#define SIMULA_FS_BLOCK_SIZE 512 /* in bytes */
#define SIMULA_FS_ENTRY_SIZE 64 /* in bytes */
#define SIMULA_FS_DATA_BLOCK_CNT ((SIMULA_FS_ENTRY_SIZE - (16 + 3 * 4)) / 4)

typedef struct kv_super_block
{
	unsigned long s_magic; /* Magic number to identify the file system */
	unsigned long s_blocksize; /* Unit of allocation */
    unsigned long s_inode_cnt; /* number of inodes */


	unsigned long partition_size; /* in blocks */
	unsigned long entry_size; /* in bytes */ 
	unsigned long entry_table_size; /* in blocks */
	unsigned long entry_table_block_start; /* in blocks */
	unsigned long entry_count; /* Total entries in the file system */
	unsigned long data_block_start; /* in blocks */
	unsigned long reserved[SIMULA_FS_BLOCK_SIZE / 4 - 8];
} KV_super_block; /* Making it of SIMULA_FS_BLOCK_SIZE */