#define KV_FS_MAGIC 0x43762103
#define DEFAULT_FILE "my_bucket"

typedef struct kv_super_block
{
    unsigned int magic;
} kv_super_block_t;

typedef struct kv_inode
{
    unsigned int entry_count;
    unsigned int sum_length;
    int saved_entry_length[256];
    // TODO
    /*
        kv_iterate를 구현하고 나면 이런 방식으로 tracking할 필요가 없을 것이다.
    */
    unsigned int flags;
} kv_inode_t;

typedef struct kv_entry
{
	char *key;
    char *value;
    // length in bytes
    int key_len;
    int value_len;
} kv_entry_t;