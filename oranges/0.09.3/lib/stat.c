#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <stdio.h>

int stat(const char* path, struct stat* buf)
{
	struct Message m;
	m.type = STAT;
	m.PATHNAME = (void*)path;
	m.BUF = (void*)buf;
	m.NAME_LEN = strlen(path);

	send_rec(BOTH, TASK_FS, &m);
	assert(m.type == SYSCALL_RET);

	return m.RETVAL;
}