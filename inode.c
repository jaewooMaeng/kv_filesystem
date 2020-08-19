#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uio.h>

#include "hkv.h"

#define FOREACH_BLK_IN_EXT(kvi, blk)					\
u32 _ix = 0, b = 0, e = 0;						\
for (_ix = 0, b = kvi->i_addrb[0], e = kvi->i_addre[0], blk = b-1;	\
_ix < KV_INODE_TSIZE;							\
++_ix, b = kvi->i_addrb[_ix], e = kvi->i_addre[_ix], blk = b-1)		\
	while (++blk < e)

void kv_destroy_inode(struct inode *inode) {
	struct kv_inode *kvi = inode->i_private;
    // 해당 inode를 가지고 와서 초기화된 inode로 바꾸어주는 것 같다.?
    // -> i_private에 대해 다시 알아볼 것
	// -> i_private는 fs or device private pointer (아직 잘 모르겠다)
	// inode 1개를 destory하는 게 아니고 kmem_cache에 저장되어있는 객체를 초기화시키는 것 같기도 하다.

	printk(KERN_INFO "#: kvfs freeing private data of inode %p (%lu)\n", kvi, inode->i_ino);
	cache_put_inode(&kvi);
}

void cache_put_inode(struct kv_inode **kvi)
{
    // put indoe가 kmem_cache를 free하는건가?
	printk(KERN_INFO "#: kvfs cache_put_inode : kvi=%p\n", *kvi);
	kmem_cache_free(kv_inode_cache, *kvi);
	// slub cache를 통해 allocate 했던 obj를 free하는 함수
	*kvi = NULL;
}

int kv_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	// inode를 만든다.
	return kv_create_inode(dir, dentry, mode);
}



void dump_kvinode(struct kv_inode *kvi)
{
	printk(KERN_INFO "----------dump_kv_inode-------------");
	printk(KERN_INFO "kv_inode addr: %p", kvi);
	printk(KERN_INFO "kv_inode->i_version: %u", kvi->i_version);
	printk(KERN_INFO "kv_inode->i_flags: %u", kvi->i_flags);
	printk(KERN_INFO "kv_inode->i_mode: %u", kvi->i_mode);
	printk(KERN_INFO "kv_inode->i_ino: %u", kvi->i_ino);
	printk(KERN_INFO "kv_inode->i_hrd_lnk: %u", kvi->i_hrd_lnk);
	printk(KERN_INFO "kv_inode->i_addrb[0]: %u", kvi->i_addrb[0]);
	printk(KERN_INFO "kv_inode->i_addre[0]: %u", kvi->i_addre[0]);
	printk(KERN_INFO "----------[end of dump]-------------");
}

struct kv_inode *cache_get_inode(void)
{
	struct kv_inode *di;

	di = kmem_cache_alloc(kv_inode_cache, GFP_KERNEL);
	printk(KERN_INFO "#: kvfs cache_get_inode : di=%p\n", di);

	return di;
}

void kv_store_inode(struct super_block *sb, struct kv_inode *kvi)
{
	struct buffer_head *bh;
	struct kv_inode *in_core;
	uint32_t blk = kvi->i_addrb[0] - 1;

	/* put in-core inode */
	/* Change me: here we just use fact that current allocator is cont.
	 * With smarter allocator the position should be found from itab
	 */
	bh = sb_bread(sb, blk);
	BUG_ON(!bh);

	in_core = (struct kv_inode *)(bh->b_data);
	memcpy(in_core, kvi, sizeof(*in_core));

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);
}

/* Here introduce allocation for directory... */
int kv_add_dir_record(struct super_block *sb, struct inode *dir,
			struct dentry *dentry, struct inode *inode)
{
	struct buffer_head *bh;
	struct kv_inode *parent, *kvi;
	struct kv_dir_entry *dir_rec;
	u32 blk, j;

	parent = dir->i_private;
	kvi = parent;

	// Find offset, in dir in extends
	FOREACH_BLK_IN_EXT(parent, blk) {
		bh = sb_bread(sb, blk);
		BUG_ON(!bh);
		dir_rec = (struct kv_dir_entry *)(bh->b_data);
		for (j = 0; j < sb->s_blocksize; ++j) {
			/* We found free space */
			if (dir_rec->inode_nr == KV_EMPTY_ENTRY) {
				dir_rec->inode_nr = inode->i_ino;
				dir_rec->name_len = strlen(dentry->d_name.name);
				memset(dir_rec->name, 0, 256);
				strcpy(dir_rec->name, dentry->d_name.name);
				mark_buffer_dirty(bh);
				sync_dirty_buffer(bh);
				brelse(bh);
				parent->i_size += sizeof(*dir_rec);
				return 0;
			}
			dir_rec++;
		}
		/* Move to another block */
		bforget(bh);
	}

	printk(KERN_ERR "Unable to put entry to directory");
	return -ENOSPC;
}

int alloc_inode(struct super_block *sb, struct kv_inode *kvi)
{
	struct kv_superblock *dsb;
	u32 i;

	dsb = sb->s_fs_info;
	dsb->s_inode_cnt += 1;
	kvi->i_ino = dsb->s_inode_cnt;
	kvi->i_version = KV_LAYOUT_VER;
	kvi->i_flags = 0;
	kvi->i_mode = 0;
	kvi->i_size = 0;

	/* TODO: check if there is any space left on the device */
	/* First block is allocated for in-core inode struct */
	/* Then 4 block for extends: that mean kvi struct is in i_addrb[0]-1 */
	kvi->i_addrb[0] = dsb->s_last_blk + 1;
	kvi->i_addre[0] = dsb->s_last_blk += 4;
	for (i = 1; i < KV_INODE_TSIZE; ++i) {
		kvi->i_addre[i] = 0;
		kvi->i_addrb[i] = 0;
	}

	kv_store_inode(sb, kvi);
	isave_intable(sb, kvi, (kvi->i_addrb[0] - 1));
	/* TODO: update inode block bitmap */

	return 0;
}

struct inode *kv_new_inode(struct inode *dir, struct dentry *dentry,
				umode_t mode)
{
	struct super_block *sb;
	struct kv_superblock *dsb;
	struct kv_inode *di;
	struct inode *inode;
	int ret;

	sb = dir->i_sb;
	dsb = sb->s_fs_info;

	di = cache_get_inode();

	/* allocate space kv way:
 	 * sb has last block on it just use it
 	 */
	ret = alloc_inode(sb, di);

	if (ret) {
		cache_put_inode(&di);
		printk(KERN_ERR "Unable to allocate disk space for inode");
		return NULL;
	}
	di->i_mode = mode;

	BUG_ON(!S_ISREG(mode) && !S_ISDIR(mode));

	/* Create VFS inode */
	inode = new_inode(sb);

	kv_fill_inode(sb, inode, di);

	/* Add new inode to parent dir */
	ret = kv_add_dir_record(sb, dir, dentry, inode);

	return inode;
}

int kv_add_ondir(struct inode *inode, struct inode *dir, struct dentry *dentry,
			umode_t mode)
{
	inode_init_owner(inode, dir, mode);
	d_add(dentry, inode);

	return 0;
}

int kv_create_inode(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct inode *inode;

	/* allocate space
	 * create incore inode
	 * create VFS inode
	 * finally ad inode to parent dir
	 */
	inode = kv_new_inode(dir, dentry, mode);

	if (!inode)
		return -ENOSPC;

	/* add new inode to parent dir */
	return kv_add_ondir(inode, dir, dentry, mode);
}

int kv_mkdir(struct inode *dir, struct dentry *dentry,
			umode_t mode)
{
	int ret = 0;

	ret = kv_create_inode(dir, dentry,  S_IFDIR | mode);

	if (ret) {
		printk(KERN_ERR "Unable to allocate dir");
		return -ENOSPC;
	}

	dir->i_op = &kv_inode_ops;
	dir->i_fop = &kv_dir_ops;

	return 0;
}

void kv_put_inode(struct inode *inode)
{
	struct kv_inode *ip = inode->i_private;

	cache_put_inode(&ip);
}

int isave_intable(struct super_block *sb, struct kv_inode *kvi, u32 i_block)
{
	struct buffer_head *bh;
	struct kv_inode *itab;
	u32 blk = 0;
	u32 *ptr;

	/* get inode table 'file' */
	bh = sb_bread(sb, KV_INODE_TABLE_OFFSET);
	itab = (struct kv_inode*)(bh->b_data);
	/* right now we just allocated one itable extend for files */
	blk = itab->i_addrb[0];
	bforget(bh);

	bh = sb_bread(sb, blk);
	/* Get block of ino inode*/
	ptr = (u32 *)(bh->b_data);
	/* inodes starts from index 1: -2 offset */
	*(ptr + kvi->i_ino - 2) = i_block;

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	return 0;
}

struct kv_inode *kv_iget(struct super_block *sb, ino_t ino)
{
	struct buffer_head *bh;
	struct kv_inode *ip;
	struct kv_inode *dinode;
	struct kv_inode *itab;
	u32 blk = 0;
	u32 *ptr;

	/* get inode table 'file' */
	bh = sb_bread(sb, KV_INODE_TABLE_OFFSET);
	itab = (struct kv_inode*)(bh->b_data);
	/* right now we just allocated one itable extend for files */
	blk = itab->i_addrb[0];
	bforget(bh);

	bh = sb_bread(sb, blk);
	/* Get block of ino inode*/
	ptr = (u32 *)(bh->b_data);
	/* inodes starts from index 1: -2 offset */
	blk = *(ptr + ino - 2);
	bforget(bh);

	bh = sb_bread(sb, blk);
	ip = (struct kv_inode*)bh->b_data;
	if (ip->i_ino == KV_EMPTY_ENTRY)
		return NULL;
	dinode = cache_get_inode();
	memcpy(dinode, ip, sizeof(*ip));
	bforget(bh);

	return dinode;
}

void kv_fill_inode(struct super_block *sb, struct inode *des, struct kv_inode *src)
{
	des->i_mode = src->i_mode;
	des->i_flags = src->i_flags;
	des->i_sb = sb;
	des->i_atime = des->i_ctime = des->i_mtime = current_time(des);
	des->i_ino = src->i_ino;
	des->i_private = src;
	des->i_op = &kv_inode_ops;

	if (S_ISDIR(des->i_mode))
		des->i_fop = &kv_dir_ops;
	else if (S_ISREG(des->i_mode))
		des->i_fop = &kv_file_ops;
	else {
		des->i_fop = NULL;
	}

	WARN_ON(!des->i_fop);
}

struct dentry *kv_lookup(struct inode *dir,
                              struct dentry *child_dentry,
                              unsigned int flags)
{
	struct kv_inode *dparent = dir->i_private;
	struct kv_inode *dchild;
	struct super_block *sb = dir->i_sb;
	struct buffer_head *bh;
	struct kv_dir_entry *dir_rec;
	struct inode *ichild;
	u32 j = 0, i = 0;

	/* Here we should use cache instead but kvfs is doing stuff in kv way.. */
	for (i = 0; i < KV_INODE_TSIZE; ++i) {
		u32 b = dparent->i_addrb[i] , e = dparent->i_addre[i];
		u32 blk = b;
		while (blk < e) {

			bh = sb_bread(sb, blk);
			BUG_ON(!bh);
			dir_rec = (struct kv_dir_entry *)(bh->b_data);

			for (j = 0; j < sb->s_blocksize; ++j) {
				if (dir_rec->inode_nr == KV_EMPTY_ENTRY) {
					break;
				}

				if (0 == strcmp(dir_rec->name, child_dentry->d_name.name)) {
					dchild = kv_iget(sb, dir_rec->inode_nr);
					ichild = new_inode(sb);
					if (!dchild) {
						return NULL;
					}
					kv_fill_inode(sb, ichild, dchild);
					inode_init_owner(ichild, dir, dchild->i_mode);
					d_add(child_dentry, ichild);

				}
				dir_rec++;
			}

			/* Move to another block */
			blk++;
			bforget(bh);
		}
	}
	return NULL;
}

