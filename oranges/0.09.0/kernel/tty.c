#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <keyboard.h>

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

static void init_tty(struct TTY* tty);
static void tty_dev_read(struct TTY* tty);
static void tty_dev_write(struct TTY* tty);
static void put_key(struct TTY* tty, unsigned int key);
static void tty_do_read(struct TTY* tty, struct Message* msg);
static void tty_do_write(struct TTY* tty, struct Message* msg);

void task_tty()
{
	//spin("TYY");
	struct TTY* tty;
	struct Message msg;
	init_keyboard();

	for (tty = TTY_FIRST; tty < TTY_END; tty++) {
		init_tty(tty);
	}
	select_console(0);

	while(1) {
		for (tty = TTY_FIRST; tty < TTY_END; tty++) {
			do {
				tty_dev_read(tty);
				tty_dev_write(tty);
			} while (tty->ibuf_cnt);
		}
		send_rec(RECEIVE, ANY, &msg);
		int src = msg.source;
		assert(src != TASK_TTY);
		struct TTY* t = &tty_table[msg.DEVICE];
		switch (msg.type) {
		case DEV_OPEN:
			reset_msg(&msg);
			msg.type = SYSCALL_RET;
			send_rec(SEND, src, &msg);
			break;
		case DEV_READ:
			tty_do_read(t, &msg);
			break;
		case DEV_WRITE:
			tty_do_write(t, &msg);
			break;
		case HARD_INT:
			key_pressed = 0;
			break;
		default:
			dump_msg("TTY::unknown message", &msg);
			break;
		}
	}

}

static void tty_do_read(struct TTY* tty, struct Message* msg)
{
	tty->tty_caller = msg->source;
	tty->tty_procnr = msg->PROC_NR;
	tty->tty_req_buf = va2la(tty->tty_procnr, msg->BUF);
	tty->tty_left_cnt = msg->CNT;
	tty->tty_trans_cnt = 0;

	msg->type = SUSPEND_PROC;
	msg->CNT = tty->tty_left_cnt;
	send_rec(SEND, tty->tty_caller, msg);
}

static void tty_do_write(struct TTY* tty, struct Message* msg)
{
	char buf[TTY_OUT_BUF_LEN];
	char* p = (char*)va2la(msg->PROC_NR, msg->BUF);
	int i = msg->CNT;
	int j;
	while (i) {
		int bytes = min(TTY_OUT_BUF_LEN, i);
		phys_copy(va2la(TASK_TTY, buf), (void*)p, bytes);
		for (j=0; j < bytes; j++) {
			out_char(tty->console, buf[j]);
		}
		i -= bytes;
		p += bytes;
	}
	msg->type = SYSCALL_RET;
	send_rec(SEND, msg->source, msg);
}

static void init_tty(struct TTY* tty)
{
	tty->ibuf_cnt = 0;
	tty->ibuf_head = tty->ibuf_tail = tty->ibuf;

	init_screen(tty);
}

static void tty_dev_read(struct TTY* tty)
{
	if (is_current_console(tty->console)) {
		keyboard_read(tty);
	}
}

static void tty_dev_write(struct TTY* tty)
{
	while (tty->ibuf_cnt) {
		char ch = *(tty->ibuf_tail);
		tty->ibuf_tail++;
		if (tty->ibuf_tail == tty->ibuf + TTY_IN_BYTES)
			tty->ibuf_tail = tty->ibuf;
		tty->ibuf_cnt--;

		if (tty->tty_left_cnt) {
			if (ch >= ' ' && ch <= '~') {
				out_char(tty->console, ch);
				void*p = tty->tty_req_buf + tty->tty_trans_cnt;
				phys_copy(p, (void*)va2la(TASK_TTY, &ch), 1);
				tty->tty_trans_cnt++;
				tty->tty_left_cnt--;
			} else if (ch == '\b' && tty->tty_trans_cnt) {
				out_char(tty->console, ch);
				tty->tty_trans_cnt--;
				tty->tty_left_cnt++;
			}

			if (ch == '\n' || tty->tty_left_cnt == 0) {
				out_char(tty->console , '\n');
				struct Message msg;
				msg.type = RESUME_PROC;
				msg.PROC_NR = tty->tty_procnr;
				msg.CNT = tty->tty_trans_cnt;
				send_rec(SEND, tty->tty_caller, &msg);
				tty->tty_left_cnt = 0;
			}
		}
	}
}

void in_process(struct TTY* tty, unsigned int key)
{
	//char output[2] = { '\0', '\0' };
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
				scroll_screen(tty->console, SCR_DN);
			}
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(tty->console, SCR_UP);
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
	if (tty->ibuf_cnt < TTY_IN_BYTES) {
		*(tty->ibuf_head) = key;
		tty->ibuf_head++;
		if (tty->ibuf_head == tty->ibuf + TTY_IN_BYTES) {
			tty->ibuf_head = tty->ibuf;
		}
		tty->ibuf_cnt++;
	}
}

// int sys_write(char* buf, int len, int edx, struct Process* p)
// {
// 	tty_write(&tty_table[p->nr_tty], buf, len);
// 	return 0;
// }

void tty_write(struct TTY* t, char* buf, int len)
{
	char* b = buf;
	int i = len;

	while (i) {
		out_char(t->console, *b++);
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
		if (c == MAG_CH_PANIC || c == MAG_CH_ASSERT) 
			continue;
		out_char(TTY_FIRST->console, c);
	}

	return 0;
}