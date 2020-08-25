/**
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/magic.h>
#endif
**/

/* Layout version */
#define KV_LAYOUT_VER		1


/* FS SIZE/OFFSET CONST */
// FS의 구조를 대충 알 수 있다.
// super - olt - inode table - bitmap - inodes ...
// KV_ROOT_IN_EXT_OFF, KV_LF_INODE_OFFSET은 뭔지 모르겠다.
#define KV_INODE_TSIZE		3
#define KV_SUPER_OFFSET		0
#define KV_DEFAULT_BSIZE	4096
#define KV_OLT_OFFSET		(KV_SUPER_OFFSET + 1)
#define KV_INODE_TABLE_OFFSET	(KV_OLT_OFFSET + 1)
#define KV_INODE_BITMAP_OFFSET	(KV_INODE_TABLE_OFFSET + KV_INODE_TABLE_SIZE + 1)
// 보통 bitmap이 먼저 오는 경우가 많은데 여기는 순서가 반대인 듯 하다.
#define KV_ROOT_INODE_OFFSET	(KV_INODE_BITMAP_OFFSET + 1)
#define KV_ROOT_IN_EXT_OFF	(KV_ROOT_INODE_OFFSET + 1)
#define KV_LF_INODE_OFFSET	(KV_ROOT_IN_EXT_OFF + KV_DEF_ALLOC)
/* Default place where FS will start using after mkfs (all above are used for mkfs) */
#define KV_FS_SPACE_START	(KV_LF_INODE_OFFSET + KV_DEF_ALLOC)

/* FS constants */
#define KV_MAGIC_NR		0x21419651
#define KV_INODE_SIZE		512
#define KV_INODE_NUMBER_TABLE	128
#define KV_INODE_TABLE_SIZE	(KV_INODE_NUMBER_TABLE * KV_INODE_SIZE)/KV_DEFAULT_BSIZE
// KV_EMPTY_INODE가 뭔지 모르겠다.
#define KV_EMPTY_INODE		0xdeeddeed

#define KV_NAME_LEN		255
#define KV_DEF_ALLOC		4	/* By default alloc N blocks per extend */

/*
 * Special inode numbers 
 */
#define KV_BAD_INO		1 /* Bad blocks inode */
#define KV_ROOT_INO		2 /* Root inode nr */
#define KV_LAF_INO		3 /* Lost and Found inode nr */

/**
 * The on-disk superblock
 */
struct kv_superblock {
	unsigned int	s_magic;	/* magic number */
	unsigned int	s_version;	/* fs version */
	unsigned int	s_blocksize;	/* fs block size */
	unsigned int	s_block_olt;	/* Object location table block */
	unsigned int	s_inode_cnt;	/* number of inodes in inode table */
	unsigned int	s_last_blk;	/* just move forward with allocation */
	// 이 extend하는 방식을 사용할지 변경할지는 고민이 필요할 것 같다.
	// list를 주고 연결하는 방식이 더 일반적인 것 같다.
};

/**
 * Object Location Table
 */
struct kv_olt {
	unsigned int	inode_table;		/* inode_table block location */
	unsigned int	inode_cnt;		/* number of inodes */
	unsigned int	inode_bitmap;		/* inode bitmap block */
};

/**
 * The on Disk inode
 */
struct kv_inode {
	unsigned int	i_version;	/* inode version */
	unsigned int	i_flags;	/* inode flags: TYPE */
	unsigned int	i_mode;		/* File mode */
	unsigned int	i_ino;		/* inode number */
	unsigned int	i_uid;		/* owner's user id */
	unsigned int	i_hrd_lnk;	/* number of hard links */
	unsigned int 	i_ctime;	/* Creation time */
	unsigned int	i_mtime;	/* Modification time*/
	unsigned int	i_size;		/* Number of bytes in file */
	/* address begin - end block, range exclusive: addres end (last block) does not belogs to extend! */
	unsigned int	i_addrb[KV_INODE_TSIZE];	/* Start block of extend ranges */
	unsigned int	i_addre[KV_INODE_TSIZE];	/* End block of extend ranges */
};

struct kv_dir_entry {
	unsigned int inode_nr;		/* inode number */
	unsigned int name_len;		/* Name length */
	char name[256];			/* File name, up to KV_NAME_LEN */
};
