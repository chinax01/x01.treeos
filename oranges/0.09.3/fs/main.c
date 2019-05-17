#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <hd.h>
#include <fs.h>
#include <stdio.h>

static void init_fs();
static void mkfs();
static void read_super_block(int dev);
static int fs_fork();
static int fs_exit();

void task_fs()
{
	printl("Task FS begins\n");
	init_fs();
	while (1) {
		send_rec(RECEIVE, ANY, &fs_msg);
		int msgtype = fs_msg.type;
		int src = fs_msg.source;
		pcaller = &proc_table[src];
		switch (msgtype) {
		case OPEN:
			fs_msg.FD = do_open();
			break;
		case CLOSE:
			fs_msg.RETVAL = do_close();
			break;
		case READ:
		case WRITE:
			fs_msg.CNT = do_rdwt();
			break;
		case UNLINK:
			fs_msg.RETVAL = do_unlink();
			break;
		case RESUME_PROC:
			src = fs_msg.PROC_NR;
			break;
		case FORK:
			fs_msg.RETVAL = fs_fork();
			break;
		case EXIT:
			fs_msg.RETVAL = fs_exit();
			break;
		case STAT:
			fs_msg.RETVAL = do_stat();
			break;
		default:
			dump_msg("FS::unkown message:", &fs_msg);
			assert(0);
			break;
		}
		if (fs_msg.type != SUSPEND_PROC) {
			fs_msg.type = SYSCALL_RET;
			send_rec(SEND, src, &fs_msg);
		}
	}
}

static void init_fs()
{
	int i;
	for (i=0; i < NR_FILE_DESC; i++) {
		memset(&f_desc_table[i], 0, sizeof(struct file_desc));
	}
	for (i=0; i < NR_INODE; i++) {
		memset(&inode_table[i], 0, sizeof(struct inode));
	}

	struct super_block * sb = super_block;
	for (; sb < &super_block[NR_SUPER_BLOCK]; sb++) {
		sb->sb_dev = NO_DEV;
	}

	struct Message msg;
	msg.type = DEV_OPEN;
	msg.DEVICE = MINOR(ROOT_DEV);
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
	send_rec(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &msg);

	mkfs();
	
	read_super_block(ROOT_DEV);

	sb = get_super_block(ROOT_DEV);
	assert(sb->magic == MAGIC_V1);

	root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}


static void mkfs()
{
	struct Message msg;
	int i, j;

	struct part_info geo;
	msg.type = DEV_IOCTL;
	msg.DEVICE = MINOR(ROOT_DEV);
	msg.REQUEST = DIOCTL_GET_GEO;
	msg.BUF = &geo;
	msg.PROC_NR = TASK_FS;
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
	send_rec(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &msg);

	printl("dev size: 0x%x sectors\n", geo.size);

	int bits_per_sect = SECTOR_SIZE * 8;
	
	// super block
	struct super_block sb;
	sb.magic = MAGIC_V1;
	sb.nr_inodes = bits_per_sect;
	sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
	sb.nr_sects = geo.size;
	sb.nr_imap_sects = 1;
	sb.nr_smap_sects = sb.nr_sects / bits_per_sect + 1;
	sb.n_1st_sect = 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
	sb.root_inode = ROOT_INODE;
	sb.inode_size = INODE_SIZE;
	struct inode x;
	sb.inode_isize_off = (int)&x.i_size - (int)&x;
	sb.inode_start_off = (int)&x.i_start_sect - (int)&x;
	sb.dir_ent_size = DIR_ENTRY_SIZE;
	struct dir_entry de;
	sb.dir_ent_inode_off = (int)&de.inode_nr - (int)&de;
	sb.dir_ent_fname_off = (int)&de.name - (int)&de;

	memset(fsbuf, 0x90, SECTOR_SIZE);
	memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

	WR_SECT(ROOT_DEV, 1);
		
	printl("devbase: 0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
		   "        inodes:0x%x00, 1st_sects:0x%x00\n",
		   geo.base * 2, (geo.base + 1) * 2, (geo.base + 2) * 2,
		   (geo.base + 2 + sb.nr_imap_sects) * 2,
		   (geo.base + 2 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
		   (geo.base + sb.n_1st_sect) * 2);
	
	memset(fsbuf, 0, SECTOR_SIZE);
	for(i = 0; i < (NR_CONSOLES + 3); i++)
		fsbuf[0] |= 1 << i;
	assert(fsbuf[0] == 0x3F);

	WR_SECT(ROOT_DEV, 2);

	// sector map
	memset(fsbuf, 0, SECTOR_SIZE);
	int nr_sects = NR_DEFAULT_FILE_SECTS + 1;
	for (i = 0; i < nr_sects / 8; i++)
		fsbuf[i] = 0xFF;
	for (j = 0; j < nr_sects % 8; j++)
		fsbuf[i] |= (1 << j);
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

	memset(fsbuf, 0, SECTOR_SIZE);
	for (i = 1; i < sb.nr_smap_sects; i++)
		WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);

	// cmd.tar
	assert(INSTALL_START_SECT + INSTALL_NR_SECTS < sb.nr_sects);
	int bit_offset = INSTALL_START_SECT - sb.n_1st_sect + 1;
	int bit_off_in_sect = bit_offset % (SECTOR_SIZE * 8);
	int bit_left = INSTALL_NR_SECTS;
	int cur_sect = bit_offset / (SECTOR_SIZE * 8);
	RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
	while (bit_left) {
		int byte_off = bit_off_in_sect / 8;
		fsbuf[byte_off] |= 1 << (bit_off_in_sect % 8);
		bit_left--;
		bit_off_in_sect++;
		if (bit_off_in_sect == (SECTOR_SIZE * 8)) {
			WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
			cur_sect++;
			RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
			bit_off_in_sect = 0;
		}
	}
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);

	// inode of '/'
	memset(fsbuf, 0, SECTOR_SIZE);
	struct inode* pi = (struct inode*) fsbuf;
	pi->i_mode = I_DIRECTORY;
	pi->i_size = DIR_ENTRY_SIZE * 5;

	pi->i_start_sect = sb.n_1st_sect;
	pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;
	
	// inode of tty0-2 
	for (i = 0; i < NR_CONSOLES; i++) {
		pi = (struct inode*)(fsbuf + (INODE_SIZE * (i+1)));
		pi->i_mode = I_CHAR_SPECIAL;
		pi->i_size = 0;
		pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
		pi->i_nr_sects = 0;
	}

	// inode of cmd.tar
	pi = (struct inode*)(fsbuf + (INODE_SIZE * (NR_CONSOLES + 1)));
	pi->i_mode = I_REGULAR;
	pi->i_size = INSTALL_NR_SECTS * SECTOR_SIZE;
	pi->i_start_sect = INSTALL_START_SECT;
	pi->i_nr_sects = INSTALL_NR_SECTS;
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

	memset(fsbuf, 0, SECTOR_SIZE);
	struct dir_entry* pde = (struct dir_entry*)fsbuf;

	pde->inode_nr = 1;
	strcpy(pde->name, ".");
	for (i = 0; i < NR_CONSOLES; i++) {
		pde++;
		pde->inode_nr = i + 2;
		sprintf(pde->name, "dev_tty%d", i);
	}
	(++pde)->inode_nr = NR_CONSOLES + 2;
	sprintf(pde->name, "cmd.tar", i);
	WR_SECT(ROOT_DEV, sb.n_1st_sect);
}

int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf)
{
	struct Message msg;
	msg.type = io_type;
	msg.DEVICE = MINOR(dev);
	msg.POSITION = pos;
	msg.BUF = buf;
	msg.CNT = bytes;
	msg.PROC_NR = proc_nr;
	assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
	send_rec(BOTH, dd_map[MAJOR(dev)].driver_nr, &msg);
	return 0;
}

struct inode* get_inode(int dev, int num)
{
	if (num == 0) return 0;

	struct inode* p;
	struct inode* q = 0;
	for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
		if (p->i_cnt) {
			if (p->i_dev == dev && p->i_num == num) {
				p->i_cnt++;
				return p;
			}
		} else {
			if (!q) {
				q = p;
			}
		}
	}
	if (!q) panic("the inode table is full.");
	q->i_dev = dev;
	q->i_num = num;
	q->i_cnt = 1;

	struct super_block* sb = get_super_block(dev);
	int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects 
		+ ((num-1) / (SECTOR_SIZE / INODE_SIZE));
	RD_SECT(dev, blk_nr);
	struct inode* pinode = (struct inode*)((u8*)fsbuf + ((num-1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE);
	q->i_mode = pinode->i_mode;
	q->i_size = pinode->i_size;
	q->i_start_sect = pinode->i_start_sect;
	q->i_nr_sects = pinode->i_nr_sects;
	return q;
}

struct super_block* get_super_block(int dev)
{
	struct super_block* sb = super_block;
	for (; sb < &super_block[NR_SUPER_BLOCK]; sb++)
		if (sb->sb_dev == dev)
			return sb;
	panic("super block of device %d not found.", dev);
	return 0;
}

void put_inode(struct inode* node)
{
	assert(node->i_cnt > 0);
	node->i_cnt--;
}

void sync_inode(struct inode* node)
{
	struct inode* p;
	struct super_block* sb = get_super_block(node->i_dev);
	int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects 
		+ ((node->i_num - 1) / (SECTOR_SIZE / INODE_SIZE));
	RD_SECT(node->i_dev, blk_nr);
	p = (struct inode*) ((u8*)fsbuf + (((node->i_num - 1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE));
	p->i_mode = node->i_mode;
	p->i_size = node->i_size;
	p->i_start_sect = node->i_start_sect;
	p->i_nr_sects = node->i_nr_sects;
	WR_SECT(node->i_dev, blk_nr);
}

static void read_super_block(int dev)
{
	int i;
	struct Message m;
	m.type = DEV_READ;
	m.DEVICE = MINOR(dev);
	m.POSITION = SECTOR_SIZE * 1;
	m.BUF = fsbuf;
	m.CNT = SECTOR_SIZE;
	m.PROC_NR = TASK_FS;
	assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
	send_rec(BOTH, dd_map[MAJOR(dev)].driver_nr, &m);

	for (i = 0; i < NR_SUPER_BLOCK; i++) {
		if (super_block[i].sb_dev == NO_DEV) break;
	}
	if (i == NR_SUPER_BLOCK)
		panic("super_block slots used up");
	assert(i == 0);
	struct super_block *sb = (struct super_block*)fsbuf;
	super_block[i] = *sb;
	super_block[i].sb_dev = dev;
}

static int fs_fork()
{
	int i;
	struct Process* child = &proc_table[fs_msg.PID];
	for (i = 0; i < NR_FILES; i++) {
		if (child->filp[i]) {
			child->filp[i]->fd_cnt++;
			child->filp[i]->fd_inode->i_cnt++;
		}
	}
	return 0;
}

static int fs_exit()
{
	int i;
	struct Process *p = &proc_table[fs_msg.PID];
	for (i = 0; i < NR_FILES; i++) {
		if (p->filp[i]) {
			p->filp[i]->fd_inode->i_cnt--;
			if (--p->filp[i]->fd_cnt == 0)
				p->filp[i]->fd_inode = 0;
			p->filp[i] = 0;
		}
	}
	return 0;
}