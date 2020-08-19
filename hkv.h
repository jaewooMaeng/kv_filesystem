#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>

#include "kv_fs.h"

extern const struct super_operations kv_sb_ops;
extern const struct inode_operations kv_inode_ops;
extern const struct file_operations kv_dir_ops;
extern const struct file_operations kv_file_ops;

extern struct kmem_cache *kv_inode_cache;

struct dentry *kvfs_mount(struct file_system_type *ft, int f, const char *dev, void *d);
int kv_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
struct dentry *kv_lookup(struct inode *dir, struct dentry *child_dentry, unsigned int flags);

/* file.c */
ssize_t kv_read(struct kiocb *iocb, struct iov_iter *to);
ssize_t kv_write(struct kiocb *iocb, struct iov_iter *from);

/* dir.c */
int kv_readdir(struct file *filp, struct dir_context *ctx);

/* inode.c */
int isave_intable(struct super_block *sb, struct kv_inode *kvi, u32 i_block);
void kv_destroy_inode(struct inode *inode);
void kv_fill_inode(struct super_block *sb, struct inode *des, struct kv_inode *src);
int kv_create_inode(struct inode *dir, struct dentry *dentry, umode_t mode);
int kv_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
void kv_store_inode(struct super_block *sb, struct kv_inode *kvi);

/* inode cache */
struct kv_inode *cache_get_inode(void);
void cache_put_inode(struct kv_inode **di);

/* super.c */
void kvfs_put_super(struct super_block *sb);
void kvfs_kill_superblock(struct super_block *sb);
