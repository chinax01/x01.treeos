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
	msg.DEVICE = MINOR(ROOT_DEV);
	assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
	send_rec(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &msg);

	spin("FS");
}