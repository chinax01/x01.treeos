#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <fs.h>
#include <stdio.h>

int do_open2()
{
	int fd = -1;
	char pathname[MAX_PATH];

	int flags = fs_msg.FLAGS;
	int name_len = fs_msg.NAME_LEN;
	int src = fs_msg.source;
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname), (void*)va2la(src, fs_msg.PATHNAME), name_len);
	pathname[name_len] = 0;

	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (pcaller->filp[i] == 0) {
			fd = i;
			break;
		}
	}
	if (fd < 0 || fd >= NR_FILES)
		panic("filp[] is full (PID: %d)", proc2pid(pcaller));

	for (i = 0; i < NR_FILE_DESC; i++) {
		if (f_desc_table[i].fd_inode == 0) break;
	}

	if (i >= NR_FILE_DESC)
		panic("f_desc_table[] is full (PID: %d)", proc2pid(pcaller));

	int inode_nr = search_file(pathname);
	
}