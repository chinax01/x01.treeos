#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int wait(int *status)
{
	struct Message m;
	m.type = WAIT;
	send_rec(BOTH, TASK_MM, &m);
	*status = m.STATUS;
	
	return (m.PID == NO_TASK ? -1 : m.PID);
}