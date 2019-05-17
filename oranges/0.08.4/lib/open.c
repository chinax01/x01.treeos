#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int open(const char* pathname, int flags)
{
	struct Message m;
	m.type = OPEN;
	m.PATHNAME = (void*)pathname;
	m.FLAGS = flags;
	m.NAME_LEN = strlen(pathname);
	send_rec(BOTH, TASK_FS, &m);
	assert(m.type == SYSCALL_RET);

	return m.FD;
}