#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "kv_fs.h"

#define KV_INODE_RATIO 0.10 /* 10% of all blocks */
#define KV_INODE_TABLE_BLOCK_START 1

KV_super_block sb =
{
	.s_magic = KV_FS_MAGIC,
	.s_block_size = KV_FS_BLOCK_SIZE,
	.inode_size = KV_FS_INODE_SIZE,
	.inode_table_block_start = KV_INODE_TABLE_BLOCK_START
};

KV_file_inode f_inode; /* All 0's */

void write_super_block(int fd, KV_super_block *sb)
{
	write(fd, sb, sizeof(KV_super_block));
    // 2번째 인자(buf)의 내용을 3번째 인자 만큼 1번째 인자에 쓴다.
}

void clear_file_inodes(int fd, KV_super_block *sb)
{
	int i;

	for (i = 0; i < sb->inode_count; i++)
	{
		write(fd, &f_inode, sizeof(f_inode));
	}
}

void mark_data_blocks(int fd, KV_super_block *sb)
{
	char c = 0;

	lseek(fd, sb->partition_size * sb->s_block_size - 1, SEEK_SET);
    // SEEK_SET은 fd의 처음을 기준으로 offset을 계산한다.
    // 파일을 mkfs를 실행하는 시점에 만드는 것처럼 partition을 만드는 것 또한, mkfs 시점에 해둔다.
    // partition 개수 만큼의 block을 file로 할당하고 마지막에 0을 써놔서 partition의 끝을 표시해둔다.
	write(fd, &c, 1); /* To make the file size to partition size */
}

int main(int argc, char *argv[])
{
	int fd;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <partition size in 512-byte blocks>\n", argv[0]);
		return 1;
	}
	sb.partition_size = atoi(argv[1]);
    // 여기서 보통 디바이스 등을 인자로 받아 이를 사용하고 mount 등을 이어나가지만, 여기서는 file을 만들고 file size를 정해버린다.
	sb.inode_table_size = sb.partition_size * KV_INODE_RATIO;
    // file 앞쪽의 이만큼을 inode table로 할당한다.
    // file의 크기에 비례하게 inode table을 할당한다.
	sb.inode_count = sb.inode_table_size * sb.s_block_size / sb.inode_size;
	sb.data_block_start = KV_INODE_TABLE_BLOCK_START + sb.inode_table_size;

	fd = creat(KV_DEFAULT_FILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    // 사용자 읽기, 사용자 쓰기, 그룹 읽기, 기타 읽기를 권한으로 주며 file을 만들며 fd를 할당받는다.

	if (fd == -1)
	{
		perror("No permissions to format");
		return 2;
	}
	write_super_block(fd, &sb);
	clear_file_inodes(fd, &sb);
	mark_data_blocks(fd, &sb);
	close(fd);
	return 0;
}