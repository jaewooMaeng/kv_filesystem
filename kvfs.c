#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/parser.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>

#include "hkv.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jaewoo");
MODULE_DESCRIPTION("KV FS");

struct file_system_type kvfs_type = {
    // name, fs_flags, read_super, file_system_type(next) 등을 가지고 있다고 알려져 있다.
	.owner = THIS_MODULE,
	.name = "kvfs",
	.mount = kvfs_mount,
	// kerenl 2.6.38 잏로 read_super(get_sb)를 mount가 대신한다.
	.kill_sb = kvfs_kill_superblock,
    // 이걸 read_super 대용으로 사용하는 것인가?
	.fs_flags = FS_REQUIRES_DEV
    // FS_REQUIRES_DEV는 block device를 사용하는 경우
};

const struct super_operations kv_sb_ops = {
	// 원래 이런 것도 inode.c에 두고 internal.h에 둔다. -> destroy 같은 함수들을 static으로 선언할 수 있다.

	// 우선 이해한 바로는 inode를 할당을 해두고 create 함수에서는 이에 대한 내용을 채우는 것 같다. x
	// -> 다시 봐야할 것 같다.
	// -> alloc의 경우 memory에 cache 형태로 inode를 올리는 것이고 create는 실제로 inode를 만드는 작업
	// -> alloc이 꼭 필요한 것은 아닐 수 있다.

	// 원 linux fs들에서 함수들은 void(*destroy_inode)(struct inode *) 이런식으로 선언이 되어있다.
	// mount할 때에 fill_super가 불리고 거기서 이 s_op들을 등록

	// inode를 할당 받거나 읽는 함수들도 있어야 하지 않을까
	// -> ex> from fs/proc/inode.c
	// .alloc_inode	= proc_alloc_inode,

	// 짜보면
	// .alloc_inode = kv_alloc_inode,
	// 이런식으로 하면 될 것이다.
	// -> 지금은 create에서 시작해 create 될 때 inode가 생성되는 식으로 되어있다.
	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //
	// -> 이렇게 되어있는 부분을 수정하면 나중에 사용하게 될 수도 있다.

	.destroy_inode = kv_destroy_inode,
	// alloc이 없기 때문에 destroy도 굳이 필요한지 모르겠다.

	// .put_super = kvfs_put_super,
	// 해당 함수의 필요성을 모르겠다.
};

const struct inode_operations kv_inode_ops = {
	// file을 만들거나 dir을 만들거나 등의 file system이 제공하는 op
	.create = kv_create,
	// kv_create -> kv_create_inode -> kv_new_inode -> alloc_inode/kv_fill_inode -> kv_add_ondir
	.mkdir = kv_mkdir,
	.lookup = kv_lookup,
};

const struct file_operations kv_file_ops = {
	// -> file_operations 같은 type은 linux/fs.h에 정의되어있다.
	// file 내용을 읽거나 쓸 때
	.read_iter = kv_read,
	.write_iter = kv_write,

	// read, write가 아니고 read_iter, write_iter를 선택한 이유?
	// -> v4 이후의 kernel에서는 read_iter를 더 많이 쓰는 것 같다.
};

const struct file_operations kv_dir_ops = {
	.owner = THIS_MODULE,
	.iterate_shared = kv_readdir,
	// file_operations에 정의되어있는 readdir 용 인자
};

struct kmem_cache *kv_inode_cache = NULL;
// slub 구조를 서언 -> indoe_cache structure만큼의 공간을 미리 할당해두고 이의 주소를 가리킨다.

static int __init kv_init(void)
{
	int ret = 0;

	kv_inode_cache = kmem_cache_create("kv_inode_cache",
					sizeof(struct kv_inode),
					0,
					(SLAB_RECLAIM_ACCOUNT| SLAB_MEM_SPREAD),
					NULL);
	// 꼭 kmem_cache를 사용해야 하는지는 잘 모르겠다 -> 성능은 좋아질 것 같다.
	// -> 필요할 것 같다.

	if (!kv_inode_cache)
		return -ENOMEM;
		// ENOMEM은 out of memory error
		// -> 왜 -를 붙이는거지?

	ret = register_filesystem(&kvfs_type);

	if (ret == 0)
		printk(KERN_INFO "Sucessfully registered kvfs\n");
	else
		printk(KERN_ERR "Failed to register kvfs. Error code: %d\n", ret);

	return ret;
}

static void __exit kv_exit(void)
{
	int ret;

	ret = unregister_filesystem(&kvfs_type);
	kmem_cache_destroy(kv_inode_cache);

	if (!ret)
		printk(KERN_INFO "Unregister kv FS success\n");
	else
		printk(KERN_ERR "Failed to unregister kv FS\n");

}

module_init(kv_init);
module_exit(kv_exit);
