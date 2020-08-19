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

struct dentry *kvfs_mount(struct file_system_type *fs_type, int flags,
                const char *dev_name, void *data)
{
	struct dentry *ret;
	ret = mount_bdev(fs_type, flags, dev_name, data, kvfs_fill_super);
    // mount_bdev는 block dev에 대한 generic한 mount 함수
	printk(KERN_INFO "#: Mounting kvfs! \n");

	if (IS_ERR(ret))
		printk(KERN_ERR "Error mounting kvfs.\n");
	else
		printk(KERN_INFO "kvfs is succesfully mounted on: %s\n", dev_name);

	return ret;
}

void kvfs_kill_superblock(struct super_block *sb)
{
	printk(KERN_INFO "#: kvfs. Unmount succesful.\n");
	kill_block_super(sb);
    // generic unmount 함수이다.
}

void kvfs_save_sb(struct super_block *sb) {
	struct buffer_head *bh;
	struct kv_superblock *d_sb = sb->s_fs_info;

	bh = sb_bread(sb, KV_SUPER_OFFSET);
	BUG_ON(!bh);

	bh->b_data = (char *)d_sb;
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);
}

static int kvfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct kv_superblock *d_sb;
	struct buffer_head *bh;
	struct inode *root_inode;
	struct kv_inode *root_kvinode, *rbuf;
	int ret = 0;

	bh = sb_bread(sb, KV_SUPER_OFFSET);
	BUG_ON(!bh);
	d_sb = (struct kv_superblock *)bh->b_data;

        sb->s_magic = d_sb->s_magic;
	sb->s_blocksize = d_sb->s_blocksize;
	sb->s_op = &kv_sb_ops;
	sb->s_fs_info = d_sb;
	bforget(bh);

	bh = sb_bread(sb, KV_ROOT_INODE_OFFSET);
	BUG_ON(!bh);

	rbuf = (struct kv_inode *)bh->b_data;
	root_kvinode = cache_get_inode();
	memcpy(root_kvinode, rbuf, sizeof(*rbuf));
	root_inode = new_inode(sb);

	/* Fill inode with kvy info */
	root_inode->i_mode = root_kvinode->i_mode;

	root_inode->i_flags = root_kvinode->i_flags;
	root_inode->i_ino = root_kvinode->i_ino;
	root_inode->i_sb = sb;
	root_inode->i_atime = current_time(root_inode);
	root_inode->i_ctime = current_time(root_inode);
	root_inode->i_mtime = current_time(root_inode);
	root_inode->i_ino = KV_ROOT_INO;
	root_inode->i_op = &kv_inode_ops;
	root_inode->i_fop = &kv_dir_ops;
	root_inode->i_private = root_kvinode;

	sb->s_root = d_make_root(root_inode);
	if (!sb->s_root) {
		ret = -ENOMEM;
		goto release;
	}

release:
	brelse(bh);
	return ret;
}

// proc에서의 fill_super

// static int proc_fill_super(struct super_block *s, struct fs_context *fc)
// {
// 	struct pid_namespace *pid_ns = get_pid_ns(s->s_fs_info);
// 	struct inode *root_inode;
// 	int ret;

// 	proc_apply_options(s, fc, pid_ns, current_user_ns());

// 	/* User space would break if executables or devices appear on proc */
// 	s->s_iflags |= SB_I_USERNS_VISIBLE | SB_I_NOEXEC | SB_I_NODEV;
// 	s->s_flags |= SB_NODIRATIME | SB_NOSUID | SB_NOEXEC;
// 	s->s_blocksize = 1024;
// 	s->s_blocksize_bits = 10;
// 	s->s_magic = PROC_SUPER_MAGIC;
// 	s->s_op = &proc_sops;
// 	s->s_time_gran = 1;

// 	/*
// 	 * procfs isn't actually a stacking filesystem; however, there is
// 	 * too much magic going on inside it to permit stacking things on
// 	 * top of it
// 	 */
// 	s->s_stack_depth = FILESYSTEM_MAX_STACK_DEPTH;
	
// 	/* procfs dentries and inodes don't require IO to create */
// 	s->s_shrink.seeks = 0;

// 	pde_get(&proc_root);
// 	root_inode = proc_get_inode(s, &proc_root);
// 	if (!root_inode) {
// 		pr_err("proc_fill_super: get root inode failed\n");
// 		return -ENOMEM;
// 	}

// 	s->s_root = d_make_root(root_inode);
// 	if (!s->s_root) {
// 		pr_err("proc_fill_super: allocate dentry failed\n");
// 		return -ENOMEM;
// 	}

// 	ret = proc_setup_self(s);
// 	if (ret) {
// 		return ret;
// 	}
// 	return proc_setup_thread_self(s);
// }

// 다음 함수의 필요성을 모르겠다.
// void kvfs_put_super(struct super_block *sb) {
// 	return;
// }
