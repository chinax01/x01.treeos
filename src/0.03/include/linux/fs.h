// include/linux/fs.h
// 定义文件系统表结构等

#ifndef __FS_H__
#define __FS_H__

#include <sys/types.h>

#define IS_BLOCKDEV(x) ((x) == 2 || (x) == 3) // 2：dev/fd; 3:dev/hd

#define READ 0
#define WRITE 1

void buffer_init(void);

#define MAJOR(a) (((unsigned)(a)) >> 8)
#define MINOR(a) ((a)&0xff)

#define NAME_LEN 14

#define I_MAP_SLOTS 8	  // i 节点槽
#define Z_MAP_SLOTS 8	  // 逻辑块槽
#define SUPER_MAGIC 0x137f // 文件系统魔数

#define NR_OPEN 20			  // 进程打开文件数
#define NR_INODE 32			  // 系统 i 节点数
#define NR_FILE 64			  // 系统文件数
#define NR_SUPER 8			  // 系统超级块数
#define NR_HASH 307			  // 缓冲区 hash 项数
#define NR_BUFFERS nr_buffers // 系统缓冲块数
#define BLOCK_SIZE 1024		  // 数据块大小

#ifndef NULL
#define NULL ((void *)0)
#endif

#define INODES_PER_BLOCK ((BLOCK_SIZE) / (sizeof(struct d_inode)))
#define DIR_ENTRIES_PER_BLOCK ((BLOCK_SIZE) / (sizeof(struct dir_entry)))

typedef char buffer_block[BLOCK_SIZE];

struct buffer_head
{
	char *b_data;
	unsigned short b_dev;
	unsigned short b_blocknr;
	unsigned char b_uptodate;
	unsigned char b_dirt;
	unsigned char b_count;
	unsigned char b_lock;
	struct task_strucdt *b_wait;
	struct buffer_head *b_prev;
	struct buffer_head *b_next;
	struct buffer_head *b_prev_free;
	struct buffer_head *b_next_free;
};

struct d_inode
{ // 磁盘 i 节点
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_time;
	unsigned char i_gid;
	unsigned char i_nlinks;
	unsigned short i_zone[9];
};

struct m_inode
{ // 内存 i 节点
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_mtime;
	unsigned char i_gid;
	unsigned char i_nlinks;
	unsigned short i_zone[9];

	struct task_struct *i_wait;
	unsigned long i_atime;
	unsigned long i_ctime;
	unsigned short i_dev;
	unsigned short i_num;
	unsigned short i_count;
	unsigned char i_lock;
	unsigned char i_dirt;
	unsigned char i_pipe;
	unsigned char i_mount;
	unsigned char i_seek;
	unsigned char i_update;
};

#define PIPE_HEAD(inode) (((long *)((inode).i_zone))[0])
#define PIPE_TAIL(inode) (((long *)((inode).i_zone))[1])
#define PIPE_SIZE(inode) ((PIPE_HEAD(inode) - PIPE_TAIL(inode)) & (PAGE_SIZE - 1))
#define PIPE_EMPTY(inode) (PIPE_HEAD(inode) == PIPE_TAIL(inode))
#define PIPE_FULL(inode) (PIPE_SIZE(inode) == (PAGE_SIZE - 1))
#define INC_PIPE(head) __asm__("incl %0\n\tandl $4095,%0" ::"m"(head))

struct file
{
	unsigned short f_mode;
	unsigned short f_flags;
	unsigned short f_count;
	struct m_inode *f_inode;
	off_t f_pos;
};

struct super_block
{
	unsigned short s_ninodes;
	unsigned short s_nzones;
	unsigned short s_imap_blocks;
	unsigned short s_zmap_blocks;
	unsigned short s_firstdatazone;
	unsigned short s_log_zone_size;
	unsigned long s_max_size;
	unsigned short s_magic;

	struct buffer_head *s_imap[8];
	struct buffer_head *s_zmap[8];
	unsigned short s_dev;
	struct m_inode *s_isup;
	struct m_inode *s_imount;
	unsigned long s_time;
	unsigned char s_rd_only;
	unsigned char s_dirt;
};

struct dir_entry
{
	unsigned short inode;
	char name[NAME_LEN];
};

extern struct m_inode inode_table[NR_INODE];
extern struct file file_table[NR_FILE];
extern struct super_block super_block[NR_SUPER];
extern struct buffer_head *start_buffer;
extern int nr_buffers;

extern void truncate(struct m_inode *inode);
extern void sync_inodes(void);
extern void wait_on(struct m_inode *inode);
extern int bmap(struct m_inode *inode, int block);
extern int create_block(struct m_inode *inode, int block);
extern struct m_inode *nmaei(const char *pathname);
extern int open_namei(const char *pathname, int flag, int mode, struct m_inode **res_inode);
extern void iput(struct m_inode *inode);
extern struct m_inode *iget(int dev, int nr);
extern struct m_inode *get_empty_inode(void);
 extern struct m_inode *get_pipe_inode(void);
extern struct buffer_head *get_hash_table(int dev, int block);
extern struct buffer_head *getblk(int dev, int block);
extern void ll_rw_block(int rw, struct buffer_head *bh);
extern void brelse(struct buffer_head *buf);
extern struct buffer_head *bread(int dev, int block);
extern int new_block(int dev);
extern void free_block(int dev, int block);
extern struct m_inode *new_inode(int dev);
extern void free_inode(struct m_inode *inode);
extern void mount_root(void);
extern struct super_block *get_super(int dev);

// extern inline struct super_block *get_super(int dev)
// {
// 	struct super_block *s;
// 	for (s = 0 + super_block; s < NR_SUPER + super_block; s++)
// 	{
// 		if (s->s_dev == dev)
// 			return s;
// 	}
// 	return NULL;
// }

#endif //__FS_H__