#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <stdio.h>

int write(int fd, const void* buf, int count)
{
	struct Message m;
	m.type = WRITE;
	m.FD = fd;
	m.BUF = (void*)buf;
	m.CNT = count;
	send_rec(BOTH, TASK_FS, &m);
	return m.CNT;
}