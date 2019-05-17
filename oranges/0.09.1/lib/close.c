#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int close(int fd)
{
	struct Message m;
	m.type = CLOSE;
	m.FD = fd;
	send_rec(BOTH, TASK_FS, &m);
	return m.RETVAL;
}