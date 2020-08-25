#include "hkv.h"

int kv_readdir(struct file *filp, struct dir_context *ctx)
{
	loff_t pos = ctx->pos;
	struct inode *inode = file_inode(filp);
	struct super_block *sb = inode->i_sb;
	struct buffer_head *bh;
	struct kv_inode *kvinode;
	struct kv_dir_entry *dir_rec;
	int j = 0, i = 0;

	kvinode = inode->i_private;

	if (pos)
		return 0;
	WARN_ON(!S_ISDIR(kvinode->i_mode));

	/* For each extends from file */
	for (i = 0; i < DM_INODE_TSIZE; ++i) {
		unsigned int b = kvinode->i_addrb[i] , e = kvinode->i_addre[i];
		unsigned int blk = b;
		while (blk < e) {

			bh = sb_bread(sb, blk);
			BUG_ON(!bh);
			dir_rec = (struct kv_dir_entry *)(bh->b_data);

			for (j = 0; j < sb->s_blocksize; j += sizeof(*dir_rec)) {
				/* We mark empty/free inodes */
				if (dir_rec->inode_nr == KV_EMPTY_INODE) {
					break;
				}
				dir_emit(ctx, dir_rec->name, dir_rec->name_len,
					dir_rec->inode_nr, DT_UNKNOWN);
                    // user_buffer에 자리가 없으면 0이 아닌 값을 return
				filp->f_pos += sizeof(*dir_rec);
				ctx->pos += sizeof(*dir_rec);
				dir_rec++;
			}

			/* Move to another block */
			blk++;
			bforget(bh);
		}
	}

	return 0;
}
