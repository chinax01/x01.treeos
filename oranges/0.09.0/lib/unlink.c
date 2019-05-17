#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <fs.h>
#include <stdio.h>

int unlink(const char* pathname)
{
	struct Message m;
	m.type = UNLINK;
	m.PATHNAME = (void*)pathname;
	m.NAME_LEN = strlen(pathname);
	send_rec(BOTH, TASK_FS, &m);
	return m.RETVAL;
}