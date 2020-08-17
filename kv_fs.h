#define KV_FS_MAGIC 0x21419651 /* Magic Number for our file system */
#define KV_FS_BLOCK_SIZE 512 /* in bytes */
#define KV_FS_INODE_SIZE 64 /* in bytes */
#define KV_FS_DATA_BLOCK_CNT ((KV_FS_INODE_SIZE - (16 + 3 * 4)) / 4)
#define KV_DEFAULT_FILE ".kvfsf" // mkfs로 VFS에 file system을 등록할 때 이런 파일을 만들고 시작한다.

typedef struct kv_super_block
{
    // file system type, block size, # of blocks, # of inodes, # of data, inode bitmap block
    // 등의 정보를 가지고 있어야 한다.
    // file  system 자체에 대한 정보
	unsigned long s_magic; /* Magic number to identify the file system */
	unsigned long s_block_size; /* Unit of allocation */
    // unsigned char s_blocksize_bits; // block이 몇 bit로 구성되어 있는지, 512 = 2^9 이므로 9bits (원래 포함)
	unsigned long partition_size; /* in blocks */
	unsigned long inode_size; /* in bytes */
	// inode의 크기는 보통 고정되어 있다.
	unsigned long inode_table_size; /* in blocks */
	unsigned long inode_table_block_start; /* in blocks */
	unsigned long inode_count; /* Total entries in the file system */
	unsigned long data_block_start; /* in blocks */
	unsigned long reserved[KV_FS_BLOCK_SIZE / 4 - 8];

    // s_version 등도 원래는 자주 포함되는 듯 하다.
    // inode_table 등을 사용 x
    // entry와 partition이 뜻하는 바가 정확하지 않다. (inode라고 바꿔버렸지만 원문에서는 entry)
    // -> entry는 한 file에 대한 정보 (inode와 대응되는 개념인 것 같다)
    // -> partition은 정말 원래 아는 partition과는 다른 뜻일 것 같다.
} KV_super_block; /* Making it of KV_FS_BLOCK_SIZE */


// 원래 오리지날 super_block

// struct super_block {
// 	struct list_head	s_list;		/* Keep this first */
// 	dev_t			s_dev;		/* search index; _not_ kdev_t */
// 	unsigned char		s_blocksize_bits;
// 	unsigned long		s_blocksize;
// 	loff_t			s_maxbytes;	/* Max file size */
// 	struct file_system_type	*s_type;
// 	const struct super_operations	*s_op;
// 	const struct dquot_operations	*dq_op;
// 	const struct quotactl_ops	*s_qcop;
// 	const struct export_operations *s_export_op;
// 	unsigned long		s_flags;
// 	unsigned long		s_iflags;	/* internal SB_I_* flags */
// 	unsigned long		s_magic;
// 	struct dentry		*s_root;
// 	struct rw_semaphore	s_umount;
// 	int			s_count;
// 	atomic_t		s_active;
// #ifdef CONFIG_SECURITY
// 	void                    *s_security;
// #endif
// 	const struct xattr_handler **s_xattr;
// #ifdef CONFIG_FS_ENCRYPTION
// 	const struct fscrypt_operations	*s_cop;
// 	struct key		*s_master_keys; /* master crypto keys in use */
// #endif
// #ifdef CONFIG_FS_VERITY
// 	const struct fsverity_operations *s_vop;
// #endif
// 	struct hlist_bl_head	s_roots;	/* alternate root dentries for NFS */
// 	struct list_head	s_mounts;	/* list of mounts; _not_ for fs use */
// 	struct block_device	*s_bdev;
// 	struct backing_dev_info *s_bdi;
// 	struct mtd_info		*s_mtd;
// 	struct hlist_node	s_instances;
// 	unsigned int		s_quota_types;	/* Bitmask of supported quota types */
// 	struct quota_info	s_dquot;	/* Diskquota specific options */

// 	struct sb_writers	s_writers;

// 	/*
// 	 * Keep s_fs_info, s_time_gran, s_fsnotify_mask, and
// 	 * s_fsnotify_marks together for cache efficiency. They are frequently
// 	 * accessed and rarely modified.
// 	 */
// 	void			*s_fs_info;	/* Filesystem private info */

// 	/* Granularity of c/m/atime in ns (cannot be worse than a second) */
// 	u32			s_time_gran;
// 	/* Time limits for c/m/atime in seconds */
// 	time64_t		   s_time_min;
// 	time64_t		   s_time_max;
// #ifdef CONFIG_FSNOTIFY
// 	__u32			s_fsnotify_mask;
// 	struct fsnotify_mark_connector __rcu	*s_fsnotify_marks;
// #endif

// 	char			s_id[32];	/* Informational name */
// 	uuid_t			s_uuid;		/* UUID */

// 	unsigned int		s_max_links;
// 	fmode_t			s_mode;

// 	/*
// 	 * The next field is for VFS *only*. No filesystems have any business
// 	 * even looking at it. You had been warned.
// 	 */
// 	struct mutex s_vfs_rename_mutex;	/* Kludge */

// 	/*
// 	 * Filesystem subtype.  If non-empty the filesystem type field
// 	 * in /proc/mounts will be "type.subtype"
// 	 */
// 	const char *s_subtype;

// 	const struct dentry_operations *s_d_op; /* default d_op for dentries */

// 	/*
// 	 * Saved pool identifier for cleancache (-1 means none)
// 	 */
// 	int cleancache_poolid;

// 	struct shrinker s_shrink;	/* per-sb shrinker handle */

// 	/* Number of inodes with nlink == 0 but still referenced */
// 	atomic_long_t s_remove_count;

// 	/* Pending fsnotify inode refs */
// 	atomic_long_t s_fsnotify_inode_refs;

// 	/* Being remounted read-only */
// 	int s_readonly_remount;

// 	/* AIO completions deferred from interrupt context */
// 	struct workqueue_struct *s_dio_done_wq;
// 	struct hlist_head s_pins;

// 	/*
// 	 * Owning user namespace and default context in which to
// 	 * interpret filesystem uids, gids, quotas, device nodes,
// 	 * xattrs and security labels.
// 	 */
// 	struct user_namespace *s_user_ns;

// 	/*
// 	 * The list_lru structure is essentially just a pointer to a table
// 	 * of per-node lru lists, each of which has its own spinlock.
// 	 * There is no need to put them into separate cachelines.
// 	 */
// 	struct list_lru		s_dentry_lru;
// 	struct list_lru		s_inode_lru;
// 	struct rcu_head		rcu;
// 	struct work_struct	destroy_work;

// 	struct mutex		s_sync_lock;	/* sync serialisation lock */

// 	/*
// 	 * Indicates how deep in a filesystem stack this SB is
// 	 */
// 	int s_stack_depth;

// 	/* s_inode_list_lock protects s_inodes */
// 	spinlock_t		s_inode_list_lock ____cacheline_aligned_in_smp;
// 	struct list_head	s_inodes;	/* all inodes */

// 	spinlock_t		s_inode_wblist_lock;
// 	struct list_head	s_inodes_wb;	/* writeback inodes */
// } __randomize_layout;

typedef struct kv_file_inode
{
	// inode에 해당되는 개념인 것 같다.
	char name[16];
	unsigned long size; /* in bytes */
	unsigned long timestamp; /* Seconds since Epoch */
	unsigned long perms; /* Permissions for user */
	unsigned long blocks[KV_FS_DATA_BLOCK_CNT];
} KV_file_inode;
