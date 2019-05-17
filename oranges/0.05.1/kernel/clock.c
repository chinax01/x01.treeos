#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

void clock_handler(int irq)
{
	ticks++;
	proc_ready->ticks--;
	
	if (k_reenter != 0) {
		return;
	}
	
	schedule();
}

void milli_delay(int ms)
{
	int t = get_ticks();
	while ( ((get_ticks() - t) * 1000 / HZ) < ms ) { }
}