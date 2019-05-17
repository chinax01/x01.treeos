#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

static void set_cursor(unsigned int pos);
static set_video_start_addr(unsigned int addr);
static void flush(struct Console * con);

void out_char(struct Console *con, char c)
{
	unsigned char* vmem = (unsigned char*)(V_MEM_BASE + con->cursor * 2);
	switch (c) {
	case '\n':
		if (con->cursor < con->original_addr + con->v_mem_limit - SCREEN_WIDTH) {
			con->cursor = con->original_addr + 
				SCREEN_WIDTH * ( (con->cursor - con->original_addr) / SCREEN_WIDTH + 1 );
		}
		break;
	case '\b':
		if (con->cursor > con->original_addr) {
			con->cursor--;
			*(vmem - 2) = ' ';
			*(vmem - 1) = DEFAULT_CHAR_COLOR;
		}
		break;
	default:
		if (con->cursor < con->original_addr + con->v_mem_limit - 1) {
			*vmem++ = c;
			*vmem++ = DEFAULT_CHAR_COLOR;
			con->cursor++;
		}
	}

	while (con->cursor >= con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(con, SCR_DN);
	}

	flush(con);
}

static void flush(struct Console * con)
{
	if (is_current_console(con)) {
		set_cursor(con->cursor);
		set_video_start_addr(con->current_start_addr);
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

void init_screen(struct TTY* tty)
{
	int nr_tty = tty - tty_table;
	tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;
	int con_v_mem_size = v_mem_size / NR_CONSOLES;
	tty->p_console->original_addr = nr_tty * con_v_mem_size;
	tty->p_console->v_mem_limit = con_v_mem_size;
	tty->p_console->current_start_addr = tty->p_console->original_addr;

	tty->p_console->cursor = tty->p_console->original_addr;
	if (nr_tty == 0) {
		tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	} else {
		out_char(tty->p_console, (nr_tty + 1) + '0');
		out_char(tty->p_console, '#');
	}

	set_cursor(tty->p_console->cursor);
}

void select_console(int nr)
{
	if (nr < 0 || nr >= NR_CONSOLES) {
		return;
	}

	nr_current_console = nr;
	set_cursor(console_table[nr].cursor);
	set_video_start_addr(console_table[nr].current_start_addr);
}

static set_video_start_addr(unsigned int addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr>>8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

void scroll_screen(struct Console* con, int direction)
{
	if (direction == SCR_UP) {
		if (con->current_start_addr > con->original_addr) {
			con->current_start_addr -= SCREEN_WIDTH;
		}
	} else if (direction == SCR_DN) {
		if (con->current_start_addr + SCREEN_SIZE < con->original_addr + con->v_mem_limit) {
			con->current_start_addr += SCREEN_WIDTH;			
		}
	}

	set_video_start_addr(con->current_start_addr);
	set_cursor(con->cursor);
}

int is_current_console(struct Console * con)
{
	return con == &console_table[nr_current_console];
}