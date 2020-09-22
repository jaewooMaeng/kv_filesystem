#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "kv_ds.h"

kv_inode_t kvi;

void kvfs_list(int kvfs_handle)
{
	char key[20];
	char value[80];
	lseek(kvfs_handle, sizeof(kv_inode_t), SEEK_SET);
	for(int i = 0; i < kvi.entry_count; i++)
	{
		memset(key, 0, 20);
		memset(value, 0, 80);
		// printf("length: %d\n", kvi.saved_entry_length[2*i]);
		// TODO
		/*
			get, put 등의 syscall 완성되면 read 등의 함수 대체
			ret = syscall(__NR_get, fd, key, keylen, val, vallen, flag);
			ret.key
			ret.val
			kv_iterstart, kv_iternext, kv_iterend 함수가 불릴 것이다.
		*/
		read(kvfs_handle, key, kvi.saved_entry_length[2*i]);
		read(kvfs_handle, value, kvi.saved_entry_length[2*i+1]);
		if (!*key) continue;
		printf("key: %s\n", key);
	}
}

void kvfs_put(int kvfs_handle, char *key, int key_len, char *value, int value_len)
{
	// TODO
	/*
		get, put 등의 syscall 완성되면 read 등의 함수 대체
		ret = syscall(__NR_put, fd, key, keylen, val, vallen, flag);
		f_op->put 구현도 필요 (syscall에서 이것을 부를 것이다)
	*/
	char prev_key[20];
	char prev_value[80];
	// printf("entry count: %d\n", kvi.entry_count);
	lseek(kvfs_handle, sizeof(kv_inode_t), SEEK_SET);
	for (int i = 0; i < kvi.entry_count; i++)
	{
		memset(prev_key, 0, 20);
		memset(prev_value, 0, 80);
		read(kvfs_handle, prev_key, kvi.saved_entry_length[2*i]);
		read(kvfs_handle, prev_value, kvi.saved_entry_length[2*i+1]);
		if (!*prev_key) break;
		if (strcmp(prev_key, key) == 0)
		{
			printf("Key %s already exists\n", key);
			return;
		}
	}
	if(2*kvi.entry_count+3 > 255)
	{
		printf("No entries left\n");
		return;
	}

	write(kvfs_handle, key, key_len);
	write(kvfs_handle, value, value_len);
	kvi.saved_entry_length[2*kvi.entry_count] = key_len;
	kvi.saved_entry_length[2*kvi.entry_count+1] = value_len;
	kvi.entry_count++;
	// printf("entry count(after):%d\n", kvi.entry_count);
	// printf("length key: %d, key_array: %d, value: %d, val_array: %d\n", key_len, value_len, kvi.saved_entry_length[2*kvi.entry_count], kvi.saved_entry_length[2*kvi.entry_count+1]);
	lseek(kvfs_handle, 0, SEEK_SET);
	write(kvfs_handle, &kvi, sizeof(kvi));
}

void kvfs_get(int kvfs_handle, char *key, int key_len, char *value, int value_len)
{
	// TODO
	/*
		get, put 등의 syscall 완성되면 read 등의 함수 대체
		ret = syscall(__NR_get, fd, key, keylen, val, vallen, flag);
		f_op->get 구현도 필요 (syscall에서 이것을 부를 것이다)
	*/
	char prev_key[20];
	char prev_value[80];
	lseek(kvfs_handle, sizeof(kv_inode_t), SEEK_SET);
	for (int i = 0; i < kvi.entry_count; i++)
	{
		memset(prev_key, 0, 20);
		memset(prev_value, 0, 80);
		read(kvfs_handle, prev_key, kvi.saved_entry_length[2*i]);
		read(kvfs_handle, prev_value, kvi.saved_entry_length[2*i+1]);
		if (!*prev_key) break;
		if (strncmp(prev_key, key, kvi.saved_entry_length[2*i]) == 0)
		{
			printf("key: %s, value: %s\n", prev_key, prev_value);
			return;
		}
	}
}

void browse_kvfs(int kv_handle)
{
	int done;
	char cmd[256], *fn;
	int ret;

	done = 0;

	printf("Welcome to Browsing Shell\n\n");

	while (!done)
	{
		printf("$> ");
		ret = scanf("%[^\n]", cmd);

		if (ret < 0)
		{
			done = 1;
			printf("\n");
			continue;
		}
		else
		{
			getchar();
			if (ret == 0) continue;
		}
		if (strcmp(cmd, "h") == 0)
		{
			printf("Supported commands:\n");
			printf("h\tquit\tls\tput\tget\n");
			continue;
		}
		else if (strcmp(cmd, "quit") == 0)
		{
			done = 1;
			continue;
		}
		else if (strcmp(cmd, "ls") == 0)
		{
			kvfs_list(kv_handle);
			continue;
		}
		else if (strncmp(cmd, "put", 3) == 0)
		{
			if (cmd[3] == ' ')
			{
				fn = cmd + 4;
				while (*fn == ' ') fn++;
				if (*fn)
				{
					char *key = strtok(fn, " ");
					int key_len = strlen(key);
					char *value;
					if (key != NULL){
						value = strtok(NULL, "");
					}
					int value_len = strlen(value);
					kvfs_put(kv_handle, key, key_len, value, value_len);
					continue;
				}
			}
		}
		else if (strncmp(cmd, "get", 3) == 0)
		{
			if (cmd[3] == ' ')
			{
				fn = cmd + 4;
				while (*fn == ' ') fn++;
				if (*fn)
				{
					char *key = strtok(fn, " ");
					int key_len = strlen(key);
					char *value;
					if (key != NULL){
						value = strtok(NULL, "");
					}
					int value_len = strlen(value);
					kvfs_get(kv_handle, key, key_len, value, value_len);
					continue;
				}
			}
		}
		printf("Unknown/Incorrect command: %s\n", cmd);
		printf("Supported commands:\n");
		printf("h\tquit\tls\tput <key, value>\tget <key>\n");
	}
}

int main(int argc, char *argv[])
{
	char *bucket = DEFAULT_FILE;
	if (argc != 2) {
        printf("Using default bucket name\n");
    } else {
        bucket = argv[1];
    }
	int kv_handle;
	kv_handle = open(bucket, O_RDWR);
	// TODO
	/*
		keyspace를 open하도록 f_op->open을 구현하고 나면 file처럼 다룰 필요 없을 것이다.
	*/
	if (kv_handle == -1)
	{
		fprintf(stderr, "Unable to browse over %s\n", bucket);
		return 2;
	}
	read(kv_handle, &kvi, sizeof(kv_inode_t));
	browse_kvfs(kv_handle);
	close(kv_handle);
	return 0;
}