#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <keyboard.h>

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

static void init_tty(struct TTY* tty);
static void tty_do_read(struct TTY* tty);
static void tty_do_write(struct TTY* tty);
static void put_key(struct TTY* tty, unsigned int key);

void task_tty()
{
	struct TTY* tty;
	init_keyboard();

	for (tty = TTY_FIRST; tty < TTY_END; tty++) {
		init_tty(tty);
	}
	select_console(0);

	while(1) {
		for (tty = TTY_FIRST; tty < TTY_END; tty++) {
			tty_do_read(tty);
			tty_do_write(tty);
		}
	}
}

static void init_tty(struct TTY* tty)
{
	tty->inbuf_count = 0;
	tty->p_inbuf_head = tty->p_inbuf_tail = tty->in_buf;

	init_screen(tty);
}

static void tty_do_read(struct TTY* tty)
{
	if (is_current_console(tty->p_console)) {
		keyboard_read(tty);
	}
}

static void tty_do_write(struct TTY* tty)
{
	if (tty->inbuf_count) {
		char c = *(tty->p_inbuf_tail);
		tty->p_inbuf_tail++;
		if (tty->p_inbuf_tail == tty->in_buf + TTY_IN_BYTES) {
			tty->p_inbuf_tail = tty->in_buf;
		}
		tty->inbuf_count--;
		out_char(tty->p_console, c);
	}
}

void in_process(struct TTY* tty, unsigned int key)
{
	char output[2] = { '\0', '\0' };
	if (!(key & FLAG_EXT)) {
		put_key(tty, key);
	} else {
		int raw_code = key & MASK_RAW;
		switch (raw_code) {
		case ENTER:
			put_key(tty, '\n');
			break;
		case BACKSPACE:
			put_key(tty, '\b');
			break;
		case UP:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(tty->p_console, SCR_DN);
			}
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(tty->p_console, SCR_UP);
			}
			break;
		case F1:
		case F2:
		case F3:
		// case F4:
		// case F5:
		// case F6:
		// case F7:
		// case F8:
		// case F9:
		// case F10:
		// case F11:
		// case F12:
			if ((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) {
				select_console(raw_code - F1);
			}
		default:
			break;
		}

	}
}

static void put_key(struct TTY* tty, unsigned int key)
{
	if (tty->inbuf_count < TTY_IN_BYTES) {
		*(tty->p_inbuf_head) = key;
		tty->p_inbuf_head++;
		if (tty->p_inbuf_head == tty->in_buf + TTY_IN_BYTES) {
			tty->p_inbuf_head = tty->in_buf;
		}
		tty->inbuf_count++;
	}
}

int sys_write(char* buf, int len, int edx, struct Process* p)
{
	tty_write(&tty_table[p->nr_tty], buf, len);
	return 0;
}

void tty_write(struct TTY* t, char* buf, int len)
{
	char* b = buf;
	int i = len;

	while (i) {
		out_char(t->p_console, *b++);
		i--;
	}
}

int sys_printx(int ebx, int ecx, char* s, struct Process* proc)
{
	const char* p;
	char c;
	char reenter_err[] = "? k_reenter is incorrect for unknown reason.";
	reenter_err[0] = MAG_CH_PANIC;

	if (k_reenter == 0)
		p = va2la(proc2pid(proc), s);
	else if (k_reenter > 0)
		p = s;
	else 
		p = reenter_err;
	
	if ( *p == MAG_CH_PANIC || (*p == MAG_CH_ASSERT && proc_ready < &proc_table[NR_TASKS]) ) {
		disable_int();
		char* v = (char*)V_MEM_BASE;
		const char* q = p + 1;
		while (v < (char*)(V_MEM_BASE + V_MEM_SIZE)) {
			*v++ = *q++;
			*v++ = RED_CHAR;
			if (!*q) {
				while ( ((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 16) ) {
					v++;
					*v++ = GRAY_CHAR;
				}
				q = p + 1;
			}
		}
		__asm__ __volatile__("hlt");
	}

	while ((c = *p++) != 0) {
		if (c = MAG_CH_PANIC || c == MAG_CH_ASSERT) 
			continue;
		out_char(tty_table[proc->nr_tty].p_console, c);
	}

	return 0;
}