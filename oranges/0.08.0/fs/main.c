#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <hd.h>

void task_fs()
{
	printl("Task FS begins\n");

	struct Message msg;
	msg.type = DEV_OPEN;
	send_rec(BOTH, TASK_HD, &msg);

	spin("FS");
}