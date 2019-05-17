#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

#define PRIVATE static
#define PUBLIC 
#define SCR_WIDTH	SCREEN_WIDTH
#define SCR_SIZE	SCREEN_SIZE

static void set_cursor(unsigned int pos);
static void set_video_start_addr(unsigned int addr);
static void flush(struct Console *con);
static void w_copy(u32 dst, const u32 src, int size);
static void clear_screen(int pos, int len);

void out_char(struct Console *con, char c)
{
	unsigned char *vmem = (unsigned char *)(V_MEM_BASE + con->cursor * 2);
	assert(con->cursor - con->orig < con->con_size);
	int cursor_x = (con->cursor - con->orig) % SCREEN_WIDTH;
	int cursor_y = (con->cursor - con->orig) / SCREEN_WIDTH;

	switch (c)
	{
	case '\n':
		con->cursor = con->orig + SCREEN_WIDTH * (cursor_y + 1);
		break;
	case '\b':
		if (con->cursor > con->orig)
		{
			con->cursor--;
			*(vmem - 2) = ' ';
			*(vmem - 1) = DEFAULT_CHAR_COLOR;
		}
		break;
	default:
		*vmem++ = c;
		*vmem++ = DEFAULT_CHAR_COLOR;
		con->cursor++;
		break;
	}

	if (con->cursor - con->orig >= con->con_size) {
		cursor_x = (con->cursor - con->orig) % SCREEN_WIDTH;
		cursor_y = (con->cursor - con->orig) / SCREEN_WIDTH;
		int cp_orig = con->orig + (cursor_y + 1) * SCREEN_WIDTH - SCREEN_SIZE;
		w_copy(con->orig, cp_orig, SCREEN_SIZE - SCREEN_WIDTH);
		con->crtc_start = con->orig;
		con->cursor = con->orig + (SCREEN_SIZE - SCREEN_WIDTH) + cursor_x;
		clear_screen(con->cursor, SCREEN_WIDTH);
		if (!con->is_full) 
			con->is_full = 1;
	}

	assert(con->cursor - con->orig < con->con_size);
	while (con->cursor >= con->crtc_start + SCREEN_SIZE 
		|| con->cursor < con->crtc_start)
	{
		scroll_screen(con, SCR_UP);
		clear_screen(con->cursor, SCREEN_WIDTH);
	}

	flush(con);
}

static void flush(struct Console *con)
{
	if (is_current_console(con))
	{
		set_cursor(con->cursor);
		set_video_start_addr(con->crtc_start);
	}
}

static void set_cursor(unsigned int pos)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (pos >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, pos & 0xFF);
	enable_int();
}

void init_screen(struct TTY *tty)
{
	int nr_tty = tty - tty_table;
	tty->console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;
	int con_size = v_mem_size / NR_CONSOLES;
	tty->console->orig = nr_tty * con_size;
	tty->console->con_size = con_size / SCREEN_WIDTH * SCREEN_WIDTH;
	tty->console->crtc_start = tty->console->cursor = tty->console->orig;
	tty->console->is_full = 0;

	if (nr_tty == 0)
	{
		tty->console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else
	{
		const char promt[] = "[TTY #?]\n";
		const char *p = promt;
		for (; *p; p++)
			out_char(tty->console, *p == '?' ? nr_tty + '0' : *p);
	}

	set_cursor(tty->console->cursor);
}

void select_console(int nr)
{
	if (nr < 0 || nr >= NR_CONSOLES)
	{
		return;
	}
	flush(&console_table[nr_current_console = nr]);
}

static void set_video_start_addr(unsigned int addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

void scroll_screen(struct Console *con, int direction)
{
	int oldest, newest, scr_top;
	newest = (con->cursor - con->orig) / SCREEN_WIDTH  * SCREEN_WIDTH;
	oldest = con->is_full ? (newest + SCREEN_WIDTH) % con->con_size : 0;
	scr_top = con->crtc_start - con->orig;

	if (direction == SCR_DN) {
		if (!con->is_full && scr_top > 0) {
			con->crtc_start -= SCREEN_WIDTH;
		} else if (con->is_full && scr_top != oldest) {
			if (con->cursor - con->orig >= con->con_size - SCREEN_SIZE) {
				if (con->crtc_start != con->orig) {
					con->crtc_start -= SCREEN_WIDTH;
				}
			} else if (con->crtc_start == con->orig) {
				scr_top = con->con_size - SCREEN_SIZE;
				con->crtc_start = con->orig + scr_top;
			} else {
				con->crtc_start -= SCREEN_WIDTH;
			}
		}
	} else if (direction == SCR_UP) {
		if (!con->is_full && newest >= scr_top + SCREEN_SIZE) {
			con->crtc_start += SCREEN_WIDTH;
		} else if (con->is_full && scr_top + SCREEN_SIZE - SCREEN_WIDTH != newest) {
			if (scr_top + SCREEN_SIZE == con->con_size)
				con->crtc_start = con->orig;
			else 
				con->crtc_start += SCREEN_WIDTH;
		}
	} else {
		assert(direction == SCR_DN || direction == SCR_UP);
	}

	flush(con);
}

int is_current_console(struct Console *con)
{
	return con == &console_table[nr_current_console];
}

static void w_copy(u32 dst, const u32 src, int size)
{
	phys_copy((void*)(V_MEM_BASE + (dst<<1)), (void*)(V_MEM_BASE + (src<<1)), size << 1);
}

static void clear_screen(int pos, int len)
{
	u8 *p = (u8*)(V_MEM_BASE + pos * 2);
	while (--len >= 0) {
		*p++ = ' ';
		*p++ = DEFAULT_CHAR_COLOR;
	}
}