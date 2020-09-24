#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/sched/xacct.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <linux/fsnotify.h>
#include <linux/security.h>
#include <linux/export.h>
#include <linux/syscalls.h>
#include <linux/pagemap.h>
#include <linux/splice.h>
#include <linux/compat.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include "internal.h"

#include <linux/uaccess.h>
#include <asm/unistd.h>

// TODO
/*
    위의 include 중 필요한 파일 추려내기
*/

/*
	새로운 파일을 만드는 것보단,
	read_wrtie에 구현되어있는 함수들을 사용할 수 있으니 read_write에 아래의 내용 추가
*/
ssize_t __vfs_kv_get(struct file *file, char __user *key, int keylen, char __user *getval, int vallen, loff_t *pos)
{
	// TODO
	// filesystem layer 등록하고나면 다음 부분 활성화
	/*
		if (file->f_op->kv_put)
			return file->f_op->kv_get(file, key, keylen, putval, vallen, pos);
		else
			return -EINVAL;
	*/
	printk(KERN_INFO "get syscall called\nkey: %s, value: %s\n", key, getval);
	return 0;
}

ssize_t vfs_kv_get(struct file *file, char __user *key, int keylen, char __user *getval, int vallen, loff_t *pos)
{
	ssize_t ret;
	ret = __vfs_kv_get(file, key, keylen, getval, vallen, pos);
	return ret;
}

ssize_t __vfs_kv_put(struct file *file, const char __user *key, int keylen, const char __user *putval, int vallen, loff_t *pos)
{
	// TODO
	// filesystem layer 등록하고나면 다음 부분 활성화
	/*
		if (file->f_op->kv_put)
			return file->f_op->kv_put(file, key, keylen, putval, vallen, pos);
		else
			return -EINVAL;
	*/
	printk(KERN_INFO "put syscall called\nkey: %s, value: %s\n", key, putval);
	return 0;
}

ssize_t vfs_kv_put(struct file *file, const char __user *key, int keylen, const char __user *putval, int vallen, loff_t *pos)
{
	ssize_t ret;

	// if (!(file->f_mode & FMODE_WRITE))
	// 	return -EBADF;
	// if (!(file->f_mode & FMODE_CAN_WRITE))
	// 	return -EINVAL;
	// if (unlikely(!access_ok(VERIFY_READ, buf, count)))
	// 	return -EFAULT;

	// ret = rw_verify_area(WRITE, file, pos, count);
	// if (!ret) {
	// 	if (count > MAX_RW_COUNT)
	// 		count =  MAX_RW_COUNT;
	// 	file_start_write(file);
	ret = __vfs_kv_put(file, key, keylen, putval, vallen, pos);
	// 	if (ret > 0) {
	// 		fsnotify_modify(file);
	// 		add_wchar(current, ret);
	// 	}
	// 	inc_syscw(current);
	// 	file_end_write(file);
	// }

	return ret;
}


SYSCALL_DEFINE6(kv_get, unsigned int, fd, char __user *, key, int, keylen, char __user *, getval, int, vallen, unsigned long, flags)
{
	struct fd f = fdget_pos(fd);
	/*
		함수 흐름:
		fdget_pos -> __fdget_pos -> __fdget -> __fget_light -> __fcheck_files -> fdtable[fd]
		자료구조 흐름:
		current(task_struct) -> files(file_struct) -> fdt(fdtable) -> fdt[fd](file)
	*/
	ssize_t ret = -EBADF;

	if (f.file) {
		loff_t pos = file_pos_read(f.file);
		/*
			호출해서 파일 포인터를 위치를 읽는다.
			TODO
			-> keyspace인 경우에 file을 읽는 것이 아니라서 bucket에도 파일 포인터가 있는 것인지 확인 필요
		*/
		ret = vfs_kv_get(f.file, key, keylen, getval, vallen, &pos);
		// TODO
		/*
			vfs_kv_get 가 정해지면 다시 정리
		*/
		if (ret >= 0)
			file_pos_write(f.file, pos);
			/*
				파일 포인터 위치를 갱신
			*/
		fdput_pos(f);
		/*
			파일 객체인 f 저장
		*/
	}
	return ret;
}

SYSCALL_DEFINE6(kv_put, unsigned int, fd, const char __user *, key, int, keylen, const char __user *, putval, int, vallen, unsigned long, flags)
{
	struct fd f = fdget_pos(fd);
	/*
		함수 흐름:
		fdget_pos -> __fdget_pos -> __fdget -> __fget_light -> __fcheck_files -> fdtable[fd]
		자료구조 흐름:
		current(task_struct) -> files(file_struct) -> fdt(fdtable) -> fdt[fd](file)
	*/
	ssize_t ret = -EBADF;

	if (f.file) {
		loff_t pos = file_pos_read(f.file);
		/*
			호출해서 파일 포인터를 위치를 읽는다.
			TODO
			-> keyspace인 경우에 file을 읽는 것이 아니라서 bucket에도 파일 포인터가 있는 것인지 확인 필요
		*/
		ret = vfs_kv_put(f.file, key, keylen, putval, vallen, &pos);
		// TODO
		/*
			vfs_kv_put 가 정해지면 다시 정리
		*/
		if (ret >= 0)
			file_pos_write(f.file, pos);
			/*
				파일 포인터 위치를 갱신
			*/
		fdput_pos(f);
		/*
			파일 객체인 f 저장
		*/
	}
	return ret;
}