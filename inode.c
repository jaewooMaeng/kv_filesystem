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
// 블록들을 훑으며 한번식 지나가는 define이다.

struct inode *kv_new_inode(struct inode *dir, struct dentry *dentry, umode_t mode);
int kv_add_ondir(struct inode *inode, struct inode *dir, struct dentry *dentry, umode_t mode);

void kv_destroy_inode(struct inode *inode) {
	struct kv_inode *kvi = inode->i_private;
    // 해당 inode를 가지고 와서 초기화된 inode로 바꾸어주는 것 같다.? x
    // -> i_private에 대해 다시 알아볼 것
	// -> i_private는 fs or device private pointer (아직 잘 모르겠다)
	// inode 1개를 destory하는 게 아니고 kmem_cache에 저장되어있는 객체를 초기화시키는 것 같기도 하다.

	printk(KERN_INFO "#: kvfs freeing private data of inode %p (%lu)\n", kvi, inode->i_ino);
	cache_put_inode(&kvi);
}

void cache_put_inode(struct kv_inode **kvi)
{
    // put indoe가 kmem_cache를 free하는건가?
	// indicate that an inode is no longer needed in memory (공식 설명 (put_inode))
	printk(KERN_INFO "#: kvfs cache_put_inode : kvi=%p\n", *kvi);
	kmem_cache_free(kv_inode_cache, *kvi);
	// slub cache를 통해 allocate 했던 obj를 free하는 함수
	*kvi = NULL;
}

int kv_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	// inode를 만든다.
	return kv_create_inode(dir, dentry, mode);
	// kv_create -> kv_create_inode -> kv_new_inode -> alloc_inode/kv_fill_inode -> kv_add_ondir
	// fs/proc에서 .lookup = proc_map_files_lookup -> proc_map_files_instantiate
	// -> proc_pid_make_inode(이게 kv_new_inode 에 해당) -> new_inode(이건 fs.h의 그것이며 여기서 alloc_inode -> sop->alloc_inode)
	// 이런식으로 전개
}

int kv_create_inode(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	// mode는 권한 같은 것을 말한다.
	struct inode *inode;

	/* allocate space
	 * create incore inode
	 * create VFS inode
	 * finally ad inode to parent dir
	 */
	inode = kv_new_inode(dir, dentry, mode);
	// proc과 같은 fs에서는 new_inode는 그냥 fs의 그것을 사용하긴 한다. (fs/inode.c)
	// -> 그 전 단계의 함수에서는 proc_pid_make_inode를 호출하긴 한다.
	// 여기서는 사실 저 함수 안에서 new_inode를 호출하긴 한다.

	if (!inode)
		return -ENOSPC;
		// 드라이브에 공간이 부족하다는 에러

	/* add new inode to parent dir */
	return kv_add_ondir(inode, dir, dentry, mode);
}

struct inode *kv_new_inode(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct super_block *sb;
	struct kv_superblock *kvsb;
	struct kv_inode *kvi;
	struct inode *inode;
	int ret;

	sb = dir->i_sb;
	kvsb = sb->s_fs_info;
	// s_fs_info는 fs에 대한 정보이다.
	// -> kvsb와 형식이 같은 것은 아닐텐데
	// -> 좀 더 알아봐야할 것 같다.
	// -> 어차피 둘 다 주소값이라 해당 file system의 정보를 담고 있는 kv_sb의 주소를 가리키면 적절할 듯하다.

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //
	// 여기서부터 원래는 new_inode -> alloc_inode -> kv_alloc_inode -> ... 이런식으로 전개가 되는 것이 맞아서
	// 여기서 하면 중복이지만, sb -> alloc_inode를 일단 등록하지 않고 이렇게 두고 나중에 수정하는 것이 나을 것 같다.
	kvi = cache_get_inode();

	/* allocate space kv way:
 	 * sb has last block on it just use it
 	 */
	ret = kv_alloc_inode(sb, kvi);
	// fs/inode.c의 alloc_inode를 사용하지 않고 여기서 새로 지정한 alloc_inode를 사용하는 모습
	// -> kv_alloc_inode를 따로 선언하고 여기서는 그렇게 사용하고 alloc_inode에 그 함수를 할당해도 될 것 같기는 하다.(알단 그렇게 해둠)
	// -> 다만 always 함수 안의 내용을 어느정도 여기서 담고 있으니 그 부분에 대해 처리해줘야 할 것이다.
	// 원본 alloc_inode는 인자로 sb만 받는다.
	// proc_get_inode -> new_inode -> alloc_inode

	if (ret) {
		// 정상적인 경우에는 0을 return 받는다.
		cache_put_inode(&kvi);
		printk(KERN_ERR "Unable to allocate disk space for inode");
		return NULL;
	}
	kvi->i_mode = mode;

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //

	BUG_ON(!S_ISREG(mode) && !S_ISDIR(mode));
	// 정규파일이나 디렉토리가 아니면 버그

	/* Create VFS inode */
	inode = new_inode(sb);
	// -> 여기서 alloc이 불리긴 한다.
	// -> 그래서

	kv_fill_inode(sb, inode, kvi);
	// 따로 함수로 빼지 않고 그냥 두는 경우가 많다.

	/* Add new inode to parent dir */
	ret = kv_add_dir_record(sb, dir, dentry, inode);

	return inode;
}

struct kv_inode *cache_get_inode(void)
{
	// kv_inode를 하나 할당 받는다.
	struct kv_inode *kvi;

	kvi = kmem_cache_alloc(kv_inode_cache, GFP_KERNEL);
	printk(KERN_INFO "#: kvfs cache_get_inode : di=%p\n", di);

	return kvi;
}

int kv_alloc_inode(struct super_block *sb, struct kv_inode *kvi)
// 원래의 alloc_inode에서는 각 sb에서 정의한 alloc_inode를 부른다.
// proc의 경우 alloc_inode에서 cache_get_inode에서 하는 kmem_cache_alloc을 실행하고 proc_inode를 채운다.
// -> 여기서는 kv_iget에서도 kmem_cache_alloc을 사용하기 때문에 일단 분리해 두엇다. (생각을 해볼 문제이다)
{
	struct kv_superblock *kvsb;
	int i;
	// u32로 선언되어 있었는데 이는 상관없을 것 같기는 하다.

	kvsb = sb->s_fs_info;
	kvsb->s_inode_cnt += 1;
	kvi->i_ino = kvsb->s_inode_cnt;
	kvi->i_version = KV_LAYOUT_VER;
	kvi->i_flags = 0;
	kvi->i_mode = 0;
	kvi->i_size = 0;

	/* TODO: check if there is any space left on the device */
	/* First block is allocated for in-core inode struct */
	/* Then 4 block for extends: that mean kvi struct is in i_addrb[0]-1 */
	kvi->i_addrb[0] = kvsb->s_last_blk + 1;
	kvi->i_addre[0] = kvsb->s_last_blk += 4;
	for (i = 1; i < KV_INODE_TSIZE; ++i) {
		kvi->i_addre[i] = 0;
		kvi->i_addrb[i] = 0;
	}
	// i_addrb, i)addre 의 [1], [2]는 값을 줄 필요가 있나?

	kv_store_inode(sb, kvi);
	// 사실상 깡통 kvi를 우선 여기서 저장시키는 것이다.(?)
	isave_intable(sb, kvi, (kvi->i_addrb[0] - 1));
	/* TODO: update inode block bitmap */

	return 0;
}

void kv_store_inode(struct super_block *sb, struct kv_inode *kvi)
{
	// kvi로 인자를 받아 버퍼로 옮겼다가 이를 저장/씀 (write out)
	struct buffer_head *bh;
	struct kv_inode *in_core;
	unsigned int blk = kvi->i_addrb[0] - 1;

	/* put in-core inode */
	/* Change me: here we just use fact that current allocator is cont.
	 * With smarter allocator the position should be found from itab
	 */
	bh = sb_bread(sb, blk);
	// sb_bread는 "Read the super block data from the device" 를 수행한다.
	// -> sb_bread(sb, block_num) 이런식으로 사용
	BUG_ON(!bh);
	// 내의 조건이 1이면 버그를 호출한다.

	in_core = (struct kv_inode *)(bh->b_data);
	memcpy(in_core, kvi, sizeof(*in_core));
	// destinatio이 in_core, 출발지가 kvi
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	// write out 실시한다.
	brelse(bh);
	// 버퍼를 free한다.
}

int isave_intable(struct super_block *sb, struct kv_inode *kvi, unsigned int i_block)
{
	struct buffer_head *bh;
	struct kv_inode *itab;
	unsigned int blk = 0;
	int *ptr;

	/* get inode table 'file' */
	bh = sb_bread(sb, KV_INODE_TABLE_OFFSET);
	itab = (struct kv_inode*)(bh->b_data);
	/* right now we just allocated one itable extend for files */
	blk = itab->i_addrb[0];
	bforget(bh);
	// brelese와 바슷히지만 potentially dirty data를 무시한다.

	bh = sb_bread(sb, blk);
	/* Get block of ino inode*/
	ptr = (int *)(bh->b_data);
	// 이게 실제로 inode가 시작되는 위치인 것 같다.
	/* inodes starts from index 1: -2 offset */
	*(ptr + kvi->i_ino - 2) = i_block;

	// 이 -2부분과 bh를 왜 2번 거치는지는 다시 봐야할 듯
	// 왜 -2...?

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	return 0;
}

void kv_fill_inode(struct super_block *sb, struct inode *des, struct kv_inode *src)
{
	// create의 경우 fill 호출하기 직전에 mode를 삽입해준다.
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
	// 잘못되면 죽이지는 않고 경고만 해준다.
}

/* Here introduce allocation for directory... */
int kv_add_dir_record(struct super_block *sb, struct inode *dir,
			struct dentry *dentry, struct inode *inode)
{
	struct buffer_head *bh;
	struct kv_inode *parent, *kvi;
	struct kv_dir_entry *dir_rec;
	unsigned int blk, j;
	// 원래 안자는 u32인데, 사실 sb_bread 같은 함수의 parameter로 들어가는 인자는 u32가 아닌 u64이긴 하다.
	// 문제 생기면 바꿔야 한다.

	parent = dir->i_private;
	kvi = parent;

	// Find offset, in dir in extends
	FOREACH_BLK_IN_EXT(parent, blk) {
		// parent의 blk 안에서 하나하나 for문을 돌려본다.
		bh = sb_bread(sb, blk);
		BUG_ON(!bh);
		dir_rec = (struct kv_dir_entry *)(bh->b_data);
		for (j = 0; j < sb->s_blocksize; ++j) {
			/* We found free space */
			if (dir_rec->inode_nr == KV_EMPTY_INODE) {
				// KV_EMPTY_INODE -> 아무것도 없는 초기 상태이며 ==이 성립하는 것 같다.
				// empty 상태의 indoe를 찾을 때까지 계속 찾다가 찾으면 이를 쓰는 방식인 것 같다.
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

int kv_add_ondir(struct inode *inode, struct inode *dir, struct dentry *dentry,
			umode_t mode)
{
	inode_init_owner(inode, dir, mode);
	d_add(dentry, inode);
	// add dentry to hash queues 라고 한다.
	// dentry에 inode를 붙인다.

	return 0;
}

// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //
// 이 위까지 create에 필요한 것들 (물론 아래에서도 위의 함수들이 필요하기도 하다.)

int kv_mkdir(struct inode *dir, struct dentry *dentry,
			umode_t mode)
{
	int ret = 0;

	ret = kv_create_inode(dir, dentry,  S_IFDIR | mode);
	// create와 같은 과정은 겪지만 S_IFDIR라는 것을 더해 dir를 만드는 것이라고 알려준다.

	if (ret) {
		printk(KERN_ERR "Unable to allocate dir");
		return -ENOSPC;
	}

	// dir->i_op = &kv_inode_ops;
	// dir->i_fop = &kv_dir_ops;

	// 이 부분은 중복이라고 생각된다.
	// -> fill_inode이 중간에 불리기 때문에
	// (변경)

	return 0;
}

// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //
// 이 위까지 mkdir에 필요한 것들 (물론 아래에서도 위의 함수들이 필요하기도 하다.)

struct dentry *kv_lookup(struct inode *dir, struct dentry *child_dentry, unsigned int flags)
{
	// 해당 dentry 파일 이름에 해당하는 inode 디렉토리를 찾는다.
	// 찾아서 여기서 뭐 하는 것 같다.
	struct kv_inode *dparent = dir->i_private;
	struct kv_inode *dchild;
	struct super_block *sb = dir->i_sb;
	struct buffer_head *bh;
	struct kv_dir_entry *dir_rec;
	struct inode *ichild;
	int j = 0, i = 0;

	/* Here we should use cache instead but kvfs is doing stuff in kv way.. */
	for (i = 0; i < KV_INODE_TSIZE; ++i) {
		unsigned int b = dparent->i_addrb[i] , e = dparent->i_addre[i];
		unsigned int blk = b;
		while (blk < e) {

			bh = sb_bread(sb, blk);
			BUG_ON(!bh);
			dir_rec = (struct kv_dir_entry *)(bh->b_data);

			for (j = 0; j < sb->s_blocksize; ++j) {
				if (dir_rec->inode_nr == KV_EMPTY_INODE) {
					break;
					// empty 뒤에는 아무것도 없기 때문에 여기까지만 뒤진다.
				}

				if (0 == strcmp(dir_rec->name, child_dentry->d_name.name)) {
					dchild = kv_iget(sb, dir_rec->inode_nr);
					ichild = new_inode(sb);
					if (!dchild) {
						return NULL;
					}
					kv_fill_inode(sb, ichild, dchild);
					inode_init_owner(ichild, dir, dchild->i_mode);
					// Init uid,gid,mode for new inode according to posix standards 라고 한다.
					d_add(child_dentry, ichild);
					// add dentry to hash queues 라고 한다.
					// child_dentry에 ichild(inode)를 붙인다.
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

struct kv_inode *kv_iget(struct super_block *sb, ino_t ino)
// iput과 반대의 함수로
// iput is the opposite of iget which searches for an inode,
// allocates memory for it if necessary and returns a reference to the inode to the caller.
// 로 설명할 수 있다.
{
	struct buffer_head *bh;
	struct kv_inode *ip;
	struct kv_inode *dinode;
	struct kv_inode *itab;
	unsigned int blk = 0;
	int *ptr;

	/* get inode table 'file' */
	bh = sb_bread(sb, KV_INODE_TABLE_OFFSET);
	itab = (struct kv_inode*)(bh->b_data);
	/* right now we just allocated one itable extend for files */
	blk = itab->i_addrb[0];
	bforget(bh);

	bh = sb_bread(sb, blk);
	/* Get block of ino inode*/
	ptr = (int *)(bh->b_data);
	/* inodes starts from index 1: -2 offset */
	blk = *(ptr + ino - 2);
	// 이것도 왜 -2인지 생각을 해봐야할 것 같다.
	bforget(bh);

	bh = sb_bread(sb, blk);
	ip = (struct kv_inode*)bh->b_data;
	if (ip->i_ino == KV_EMPTY_INODE)
		return NULL;
	dinode = cache_get_inode();
	memcpy(dinode, ip, sizeof(*ip));
	bforget(bh);

	return dinode;
}

// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //
// 이 위까지는 쓰임새를 발견 아래는 일단 두는 함수들
// 마치 뺄셈을 쓰지 않아도 +가 있으면 -가 있는 것처럼
void kv_put_inode(struct inode *inode)
{
	// 이건 그냥 iget의 반대 개념의 함수로 일단 남겨둔다.
	// -> 필요는 없는 것 같다.
	struct kv_inode *ip = inode->i_private;

	cache_put_inode(&ip);
}















// void dump_kvinode(struct kv_inode *kvi)
// 우선 쓰임새를 아직 잘 모르겠어서 뺀다.
// {
// 	printk(KERN_INFO "----------dump_kv_inode-------------");
// 	printk(KERN_INFO "kv_inode addr: %p", kvi);
// 	printk(KERN_INFO "kv_inode->i_version: %u", kvi->i_version);
// 	printk(KERN_INFO "kv_inode->i_flags: %u", kvi->i_flags);
// 	printk(KERN_INFO "kv_inode->i_mode: %u", kvi->i_mode);
// 	printk(KERN_INFO "kv_inode->i_ino: %u", kvi->i_ino);
// 	printk(KERN_INFO "kv_inode->i_hrd_lnk: %u", kvi->i_hrd_lnk);
// 	printk(KERN_INFO "kv_inode->i_addrb[0]: %u", kvi->i_addrb[0]);
// 	printk(KERN_INFO "kv_inode->i_addre[0]: %u", kvi->i_addre[0]);
// 	printk(KERN_INFO "----------[end of dump]-------------");
// }

