#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

void task_sys()
{
	struct Message m;
	while (1) {
		send_rec(RECEIVE, ANY, &m);
		int src = m.source;
		switch (m.type) {
		case GET_TICKS:
			m.RETVAL = ticks;
			send_rec(SEND, src, &m);
			break;
		case GET_PID:
			m.type = SYSCALL_RET;
			m.PID = src;
			send_rec(SEND, src, &m);
			break;
		default:
			panic("Unknown message type.");
			break;
		}
	}
}