#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

void task_sys()
{
	struct Message msg;
	while (1) {
		send_rec(RECEIVE, ANY, &msg);
		int src = msg.source;

		switch (msg.type) {
		case GET_TICKS:
			msg.RETVAL = ticks;
			send_rec(SEND, src, &msg);
			break;
		default:
			panic("unknown msg type");
			break;
		}
	}
}

void task_sys2()
{
	
	struct Message m;
	while (1) {
		printf("In task_sys().\n");
		send_rec(RECEIVE, ANY, &m);
		int src = m.source;
		switch (m.type) {
		case GET_TICKS:
			m.RETVAL = ticks;
			send_rec(SEND, src, &m);
			break;
		default:
			panic("Unknown message type.");
			break;
		}
	}
}