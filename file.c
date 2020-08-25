#include <linux/uio.h>

#include "hkv.h"

ssize_t kv_read(struct kiocb *iocb, struct iov_iter *to)
{
	struct super_block *sb;
	struct inode *inode;
	struct kv_inode *kvinode;
	struct buffer_head *bh;
	char *buffer;
	void *buf = to->iov->iov_base;
    // iov는 데이터를 buf로 모아서 전송하는 구조체
	// iov_base는 전송할 데이터의 시작주소
	int nbytes;
	size_t count = iov_iter_count(to);
    // iov_iter_count는 to의 count를 return
    // 이 count가 무슨 뜻인지는 좀 더 알아봐야 한다.
	loff_t off = iocb->ki_pos;
	loff_t end = off + count;
	size_t blk = 0;


	inode = iocb->ki_filp->f_path.dentry->d_inode;
    // 이런식으로 사용함
    // filep에서 이런식으로 찾아간다고 함
	sb = inode->i_sb;
	kvinode = inode->i_private;
	if (off) {
		return 0;
	}

	/* calculate datablock number here */
	blk = kv_get_loffset(kvinode, off);
	bh = sb_bread(sb, blk);
	if (!bh) {
        	printk(KERN_ERR "Failed to read data block %lu\n", blk);
		return 0;
	}

	buffer = (char *)bh->b_data + (off % DM_DEFAULT_BSIZE);
	nbytes = min((size_t)(kvinode->i_size - off), count);
    // 나름 우선 해석을 하자면 nbytes는 거기까지 읽는다는 것 같다.

	if (copy_to_user(buf, buffer, nbytes)) {
        // copy_to_user는 buffer의 내용을 nbytes만큼 buf에 복사
        // 정상적으로 되었을 때 0을 return한다.
		brelse(bh);
		printk(KERN_ERR
			"Error copying file content to userspace buffer\n");
		return -EFAULT;
	}

	brelse(bh);
	iocb->ki_pos += nbytes;

	return nbytes;
}

ssize_t kv_get_loffset(struct kv_inode *kvi, loff_t off)
{
    // offset 값과 kv_inode를 사용해 blk(위치)를 찾아내는 함수
	ssize_t ret = KV_EMPTY_INODE;
	loff_t add = 0;
	int i = 0;

	if (off > DM_DEFAULT_BSIZE)
		add += DM_DEFAULT_BSIZE % off;

	for (i = 0; i < DM_INODE_TSIZE; ++i) {
		if (kvi->i_addrb[i] + off > kvi->i_addre[i]) {
            // 이런 경우가 발생할 수 있는 것인가?
			off -= (kvi->i_addre[i] - kvi->i_addrb[i]);
		} else {
			ret = kvi->i_addrb[i] + off;
			break;
		}
	}

	BUG_ON(ret == 0xdeadbeef);
    // 왜 KV_EMPTY_INODE의 값이 아니고 다른 값일 때 에러인 것인가?

	return ret;
}

// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //
// 위에까지 read에 필요한 내용

ssize_t kv_write(struct kiocb *iocb, struct iov_iter *from)
{
	struct super_block *sb;
	struct inode *inode;
	struct kv_inode *kvinode;
	struct buffer_head *bh;
	struct kv_superblock *kvsb;
	void *buf = from->iov->iov_base; 
	loff_t off = iocb->ki_pos;
	size_t count = iov_iter_count(from);
	size_t blk = 0;	
	size_t boff = 0;
	char *buffer;
	int ret;

	inode = iocb->ki_filp->f_path.dentry->d_inode;
	sb = inode->i_sb;
	kvinode = inode->i_private;
	kvsb = sb->s_fs_info;
	
	ret = generic_write_checks(iocb, from);
    /*
    * generic_write_checks
    * ->
    * Performs necessary checks before doing a write
    *
    * Can adjust writing position or amount of bytes to write.
    * Returns appropriate error code that caller should return or
    * zero in case that write should be allowed.
    * 라고 한다.
    */
	if (ret <= 0) {
	    printk(KERN_INFO "KVFS: generic_write_checks Failed: %d", ret); 
	    return ret;
	}
	
	/* calculate datablock to write alloc if necessary */
	// blk = kv_alloc_if_necessary(kvsb, kvinode, off, count);
    // 현재 아무것도 하지 않기 때문에 일단 빼도 될 것 같다. -> 나중에 제대로 구현하면 다시 넣어도 되겠다.

	/* kv files are contigous so offset can be easly calculated */
	boff = kv_get_loffset(kvinode, off);
	bh = sb_bread(sb, boff);
	if (!bh) {
	    printk(KERN_ERR "Failed to read data block %lu\n", blk);
	    return 0;
	}
	
	buffer = (char *)bh->b_data + (off % DM_DEFAULT_BSIZE);
	if (copy_from_user(buffer, buf, count)) {
        // buf의 내용을 buffer에 복사
	    brelse(bh);
	    printk(KERN_ERR
	           "Error copying file content from userspace buffer "
	           "to kernel space\n");
	    return -EFAULT;
	}
	
	iocb->ki_pos += count; 
	
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);
    // 여기 빼도 작동할 것 같지만 우선 놔두겠다.
	
	kvinode->i_size = max((size_t)(kvinode->i_size), count);

	kv_store_inode(sb, kvinode);
    // 이러면 위의 mark_buffer_dirty와 sync_dirty_buffer가 중복이 되는 거 아닌가?
	
	return count;
}

// ssize_t kv_alloc_if_necessary(struct kv_superblock *sb, struct kv_inode *kvi,
// 				loff_t off, size_t cnt)
// {
// 	// Mock it until using bitmap
// 	return 0;
// }





// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ //
// fs/proc에서 사용하는 read 중 하나
// 대충 비슷한 과정을 거친다. count 찾고 buf 정해서 copy_to_user

// /**
//  *	seq_read -	->read() method for sequential files.
//  *	@file: the file to read from
//  *	@buf: the buffer to read to
//  *	@size: the maximum number of bytes to read
//  *	@ppos: the current position in the file
//  *
//  *	Ready-made ->f_op->read()
//  */
// ssize_t seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
// {
// 	struct seq_file *m = file->private_data;
// 	size_t copied = 0;
// 	size_t n;
// 	void *p;
// 	int err = 0;

// 	mutex_lock(&m->lock);

// 	/*
// 	 * if request is to read from zero offset, reset iterator to first
// 	 * record as it might have been already advanced by previous requests
// 	 */
// 	if (*ppos == 0) {
// 		m->index = 0;
// 		m->count = 0;
// 	}

// 	/* Don't assume *ppos is where we left it */
// 	if (unlikely(*ppos != m->read_pos)) {
// 		while ((err = traverse(m, *ppos)) == -EAGAIN)
// 			;
// 		if (err) {
// 			/* With prejudice... */
// 			m->read_pos = 0;
// 			m->index = 0;
// 			m->count = 0;
// 			goto Done;
// 		} else {
// 			m->read_pos = *ppos;
// 		}
// 	}

// 	/* grab buffer if we didn't have one */
// 	if (!m->buf) {
// 		m->buf = seq_buf_alloc(m->size = PAGE_SIZE);
// 		if (!m->buf)
// 			goto Enomem;
// 	}
// 	/* if not empty - flush it first */
// 	if (m->count) {
// 		n = min(m->count, size);
// 		err = copy_to_user(buf, m->buf + m->from, n);
// 		if (err)
// 			goto Efault;
// 		m->count -= n;
// 		m->from += n;
// 		size -= n;
// 		buf += n;
// 		copied += n;
// 		if (!size)
// 			goto Done;
// 	}
// 	/* we need at least one record in buffer */
// 	m->from = 0;
// 	p = m->op->start(m, &m->index);
// 	while (1) {
// 		err = PTR_ERR(p);
// 		if (!p || IS_ERR(p))
// 			break;
// 		err = m->op->show(m, p);
// 		if (err < 0)
// 			break;
// 		if (unlikely(err))
// 			m->count = 0;
// 		if (unlikely(!m->count)) {
// 			p = m->op->next(m, p, &m->index);
// 			continue;
// 		}
// 		if (m->count < m->size)
// 			goto Fill;
// 		m->op->stop(m, p);
// 		kvfree(m->buf);
// 		m->count = 0;
// 		m->buf = seq_buf_alloc(m->size <<= 1);
// 		if (!m->buf)
// 			goto Enomem;
// 		p = m->op->start(m, &m->index);
// 	}
// 	m->op->stop(m, p);
// 	m->count = 0;
// 	goto Done;
// Fill:
// 	/* they want more? let's try to get some more */
// 	while (1) {
// 		size_t offs = m->count;
// 		loff_t pos = m->index;

// 		p = m->op->next(m, p, &m->index);
// 		if (pos == m->index) {
// 			pr_info_ratelimited("buggy seq_file .next function %ps "
// 				"did not updated position index\n",
// 				m->op->next);
// 			m->index++;
// 		}
// 		if (!p || IS_ERR(p)) {
// 			err = PTR_ERR(p);
// 			break;
// 		}
// 		if (m->count >= size)
// 			break;
// 		err = m->op->show(m, p);
// 		if (seq_has_overflowed(m) || err) {
// 			m->count = offs;
// 			if (likely(err <= 0))
// 				break;
// 		}
// 	}
// 	m->op->stop(m, p);
// 	n = min(m->count, size);
// 	err = copy_to_user(buf, m->buf, n);
// 	if (err)
// 		goto Efault;
// 	copied += n;
// 	m->count -= n;
// 	m->from = n;
// Done:
// 	if (!copied)
// 		copied = err;
// 	else {
// 		*ppos += copied;
// 		m->read_pos += copied;
// 	}
// 	mutex_unlock(&m->lock);
// 	return copied;
// Enomem:
// 	err = -ENOMEM;
// 	goto Done;
// Efault:
// 	err = -EFAULT;
// 	goto Done;
// }
// EXPORT_SYMBOL(seq_read);