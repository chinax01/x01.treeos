#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <fs.h>
#include <stdio.h>

static struct inode *create_file(char *path, int flags);
static int alloc_imap_bit(int dev);
static int alloc_smap_bit(int dev, int nr_sects_to_alloc);
static struct inode* new_inode(int dev, int inode_nr, int start_sect);
static void new_dir_entry(struct inode* dir_inode, int inode_nr, char* filename);


int do_open()
{
	int fd = -1;
	char pathname[MAX_PATH];

	int flags = fs_msg.FLAGS;
	int name_len = fs_msg.NAME_LEN;
	int src = fs_msg.source;
	assert(name_len < MAX_PATH);
	phys_copy((void *)va2la(TASK_FS, pathname), (void *)va2la(src, fs_msg.PATHNAME), name_len);
	pathname[name_len] = 0;

	int i;
	for (i = 0; i < NR_FILES; i++)
	{
		if (pcaller->filp[i] == 0)
		{
			fd = i;
			break;
		}
	}
	if (fd < 0 || fd >= NR_FILES)
		panic("filp[] is full (PID: %d)", proc2pid(pcaller));

	for (i = 0; i < NR_FILE_DESC; i++)
	{
		if (f_desc_table[i].fd_inode == 0)
			break;
	}

	if (i >= NR_FILE_DESC)
		panic("f_desc_table[] is full (PID: %d)", proc2pid(pcaller));

	int inode_nr = search_file(pathname);
	struct inode *pin = 0;
	if (flags & O_CREAT)
	{
		if (inode_nr)
		{
			printl("file exists.\n");
			return -1;
		}
		else
		{
			pin = create_file(pathname, flags);
		
		}
	}
	else
	{
		assert(flags & O_RDWR);
		char filename[MAX_PATH];
		struct inode *dir_inode;
		if (strip_path(filename, pathname, &dir_inode) != 0)
		{
			return -1;
		}
		pin = get_inode(dir_inode->i_dev, inode_nr);
	}

	if (pin)
	{
		pcaller->filp[fd] = &f_desc_table[i];
		f_desc_table[i].fd_inode = pin;
		f_desc_table[i].fd_mode = flags;
		f_desc_table[i].fd_cnt = 1;
		f_desc_table[i].fd_pos = 0;
		int imode = pin->i_mode & I_TYPE_MASK;
		if (imode == I_CHAR_SPECIAL) {
			struct Message m;
			m.type = DEV_OPEN;
			int dev = pin->i_start_sect;
			m.DEVICE = MINOR(dev);
			assert(MAJOR(dev) == 4);
			assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
			send_rec(BOTH, dd_map[MAJOR(dev)].driver_nr, &m);
		} else if (imode == I_DIRECTORY) {
			assert(pin->i_num == ROOT_INODE);
		} else {
			assert(pin->i_mode == I_REGULAR);
		}
	} else {
		return -1;
	}

	return fd;
}

static struct inode * create_file(char *path, int flags)
{
	char filename[MAX_PATH];
	struct inode *dir_inode;
	if (strip_path(filename, path, &dir_inode) != 0)
		return 0;
	int inode_nr = alloc_imap_bit(dir_inode->i_dev);
	int free_sect_nr = alloc_smap_bit(dir_inode->i_dev, NR_DEFAULT_FILE_SECTS);
	struct inode *newino = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr);
	new_dir_entry(dir_inode, newino->i_num, filename);
	return newino;
}


static int alloc_imap_bit(int dev)
{
	int inode_nr = 0;
	int i, j, k;
	int imap_blk0_nr = 1 + 1;
	struct super_block *sb = get_super_block(dev);
	for (i = 0; i < sb->nr_imap_sects; i++)
	{
		RD_SECT(dev, imap_blk0_nr + i);
		for (j = 0; j < SECTOR_SIZE; j++)
		{
			if (fsbuf[j] == 0xFF)
				continue;
			for (k = 0; ((fsbuf[j] >> k) & 1) != 0; k++)
			{
			}
			inode_nr = (i * SECTOR_SIZE + j) * 8 + k;
			fsbuf[j] |= (1 << k);
			WR_SECT(dev, imap_blk0_nr + i);
			break;
		}
		return inode_nr;
	}
	panic("inode-map is probably full.\n");
	return 0;
}


static int alloc_smap_bit(int dev, int nr_sects_to_alloc)
{
	int i, j, k;
	struct super_block *sb = get_super_block(dev);
	int smap_blk0_nr = 1 + 1 + sb->nr_imap_sects;
	int free_sect_nr = 0;
	for (i = 0; i < sb->nr_smap_sects; i++)
	{
		RD_SECT(dev, smap_blk0_nr + i);
		for (j = 0; j < SECTOR_SIZE && nr_sects_to_alloc > 0; j++)
		{
			k = 0;
			if (!free_sect_nr)
			{
				if (fsbuf[j] == 0xFF)
					continue;
				for (; ((fsbuf[j] >> k) & 1) != 0; k++)
				{
				}
				free_sect_nr = (i * SECTOR_SIZE + j) * 8 + k - 1 + sb->n_1st_sect;
			}
			for (; k < 8; k++)
			{
				assert(((fsbuf[j] >> k) & 1) == 0);
				fsbuf[j] |= (1 << k);
				if (--nr_sects_to_alloc == 0)
					break;
			}
		}
		if (free_sect_nr)
			WR_SECT(dev, smap_blk0_nr + i);
		if (nr_sects_to_alloc == 0)
			break;
	}
	assert(nr_sects_to_alloc == 0);
	return free_sect_nr;
}

static struct inode* new_inode(int dev, int inode_nr, int start_sect)
{
	struct inode* node = get_inode(dev, inode_nr);
	node->i_mode = I_REGULAR;
	node->i_size = 0;
	node->i_start_sect = start_sect;
	node->i_nr_sects = NR_DEFAULT_FILE_SECTS;
	node->i_dev = dev;
	node->i_cnt = 1;
	node->i_num = inode_nr;
	sync_inode(node);
	return node;
}

static void new_dir_entry(struct inode* dir_inode, int inode_nr, char* filename)
{
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
	int m = 0;
	struct dir_entry* pde;
	struct dir_entry* new_de = 0;

	int i,j;
	for (i=0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry*)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
			if (++m > nr_dir_entries) break;
			if (pde->inode_nr == 0) {
				new_de = pde;
				break;
			}
		}
		if (m > nr_dir_entries || new_de) break;
	}
	if (!new_de) {
		new_de = pde;
		dir_inode->i_size += DIR_ENTRY_SIZE;
	}
	new_de->inode_nr = inode_nr;
	strcpy(new_de->name, filename);
	WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);
	sync_inode(dir_inode);
}

int do_close()
{
	int fd = fs_msg.FD;
	put_inode(pcaller->filp[fd]->fd_inode);
	if (--pcaller->filp[fd]->fd_cnt == 0)
		pcaller->filp[fd]->fd_inode = 0;
	pcaller->filp[fd] = 0;
	
	return 0;
}