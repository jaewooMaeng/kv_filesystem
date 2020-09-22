#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// #include <linux/fs.h>

#include "kv_ds.h"

kv_super_block_t kvsb =
{
	.magic = KV_FS_MAGIC,
};

kv_inode_t kvi =
{
    .entry_count = 0,
    .sum_length = 0
};

int main(int argc, char *argv[])
{
    int kvfs_handle;
    char *bucket = DEFAULT_FILE;
    if (argc != 2) {
        printf("Using default bucket name\n");
    } else {
        bucket = argv[1];
    }
    kvfs_handle = open(bucket, O_RDWR | O_CREAT);
    write(kvfs_handle, &kvi, sizeof(kvi));
    close(kvfs_handle);
    return 0;
}