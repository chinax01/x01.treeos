#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <hd.h>
#include <fs.h>

static void init_fs();
static void mkfs();

void task_fs()
{
	printl("Task FS begins\n");
	init_fs();
	spin("FS");
	// while (1) {
	// 	send_rec(RECEIVE, ANY, &fs_msg);
	// 	int src = fs_msg.source;
	// 	pcaller = &proc_table[src];
	// 	switch (fs_msg.type) {
	// 	case OPEN:
	// 	spin("\nOK!");
	// 		fs_msg.FD = do_open();
	// 		break;
	// 	case CLOSE:
	// 		//fs_msg.RETVAL = do_close();
	// 		break;
	// 	default:
	// 		dump_msg("FS::unkown message:", &fs_msg);
	// 		assert(0);
	// 		break;
	// 	}
	// 	fs_msg.type = SYSCALL_RET;
	// 	send_rec(SEND, src, &fs_msg);
	// }
}

static void init_fs()
{
	struct Message msg;
	msg.type = DEV_OPEN;
	msg.DEVICE = MINOR(ROOT_DEV);
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
	send_rec(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &msg);

	mkfs();
}

static void mkfs()
{
	struct Message msg;
	int i, j;
	int bits_per_sect = SECTOR_SIZE * 8;

	struct part_info geo;
	msg.type = DEV_IOCTL;
	msg.DEVICE = MINOR(ROOT_DEV);
	msg.REQUEST = DIOCTL_GET_GEO;
	msg.BUF = &geo;
	msg.PROC_NR = TASK_FS;
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
	send_rec(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &msg);

	printl("dev size: 0x%x sectors\n", geo.size);

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
// spin("\nOK");
	// inode map
	memset(fsbuf, 0, SECTOR_SIZE);
	for(i = 0; i < (NR_CONSOLES + 2); i++)
		fsbuf[0] |= 1 << i;
	assert(fsbuf[0] == 0x1F);

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

	// inodes
	memset(fsbuf, 0, SECTOR_SIZE);
	struct inode* pi = (struct inode*) fsbuf;
	pi->i_mode = I_DIRECTORY;
	pi->i_size = DIR_ENTRY_SIZE * 4;

	pi->i_start_sect = sb.n_1st_sect;
	pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;
	for (i = 0; i < NR_CONSOLES; i++) {
		pi = (struct inode*)(fsbuf + (INODE_SIZE * (i+1)));
		pi->i_mode = I_CHAR_SPECIAL;
		pi->i_size = 0;
		pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
		pi->i_nr_sects = 0;
	}
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