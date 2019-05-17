#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

void exit(int status)
{
	struct Message m;
	m.type = EXIT;
	m.STATUS = status;
	send_rec(BOTH, TASK_MM, &m);
	assert(m.type == SYSCALL_RET);
}