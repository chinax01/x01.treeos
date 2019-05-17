#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int fork()
{
	struct Message m;
	m.type = FORK;
	send_rec(BOTH, TASK_MM, &m);
	assert(m.type == SYSCALL_RET);
	assert(m.RETVAL == 0);
	
	return m.PID;
}