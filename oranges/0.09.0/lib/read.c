#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <stdio.h>

int read(int fd, void* buf, int count)
{
	struct Message m;
	m.type = READ;
	m.FD = fd;
	m.BUF = buf;
	m.CNT = count;
	send_rec(BOTH, TASK_FS, &m);
	return m.CNT;
}