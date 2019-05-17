#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

void init_clock()
{
	out_byte( TIMER_MODE, RATE_GENERATOR );
	out_byte( TIMER0, (unsigned char)(TIMER_FREQ / HZ) );
	out_byte( TIMER0, (unsigned char)((TIMER_FREQ / HZ) >> 8) );
	
	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);


}

void clock_handler(int irq)
{
	if (++ticks >= MAX_TICKS) ticks = 0;

	if (proc_ready->ticks)
		proc_ready->ticks--;
	
	if (key_pressed)
		inform_int(TASK_TTY);
		
	if (k_reenter != 0) {
		return;
	}
	if (proc_ready->ticks > 0) {
		return;
	}
	
	schedule();
}

void milli_delay(int ms)
{
	int t = get_ticks();
	while ( ((get_ticks() - t) * 1000 / HZ) < ms ) { }
}