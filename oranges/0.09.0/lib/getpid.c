#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int getpid()
{
	struct Message m;
	m.type = GET_PID;
	send_rec(BOTH, TASK_SYS, &m);
	assert(m.type == SYSCALL_RET);

	return m.PID;
}