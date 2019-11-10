#include <linux/sched.h>
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>

#define SCREEN_START 0xb8000
#define SCREEN_END 0xc0000
#define LINES 25
#define COLUMNS 80
#define NPAR 16

extern void keyboard_interrupt(void);

// 对终端 ESC-Z 或 csi0c 请求的应答，‘ESC [?1:2c' 表示兼容高级视频功能的 VT100
#define RESPONSE "\033[?1;2c"

#define CURSOR_X (*(unsigned char *)0x90000)
#define CURSOR_Y (*(unsigned char *)0x90001)

static unsigned long origin = SCREEN_START;
static unsigned long scr_end = SCREEN_START + LINES * COLUMNS * 2;
static unsigned long pos;
static unsigned long x, y;
static unsigned long top = 0, bottom = LINES;
static unsigned long lines = LINES, columns = COLUMNS;
static unsigned long state = 0;
static unsigned long npar, par[NPAR];
static unsigned long ques = 0;
static unsigned char attr = 0x07;

static void puts(const char *);
static char *vidmem = (char *)0xb8000;
static int vidport;
static int lines_1 = 25, cols = 80;

static inline void gotoxy(unsigned int new_x, unsigned int new_y)
{
	if (new_x >= columns || new_y >= lines)
		return;
	x = new_x;
	y = new_y;
	pos = origin + ((y * columns + x) << 1);
}

static inline void set_origin(void)
{
	cli();
	outb_p(12, 0x3d4);
	outb_p(0xff & ((origin - SCREEN_START) >> 9), 0x3d5);
	outb_p(13, 0x3d4);
	outb_p(0xff & ((origin - SCREEN_START) >> 1), 0x3d5);
	sti();
}

static void scrup(void)
{
	if (!top && bottom == lines)
	{
		origin += columns << 1;
		pos += columns << 1;
		scr_end += columns << 1;
		if (scr_end > SCREEN_END)
		{
			__asm__("cld\n\t"
					"rep\n\t"
					"movsl\n\t" // esi => edi
					"movl columns, %1\n\t"
					"rep\n\t"
					"stosw" ::"a"(0x0720), // erase char
					"c"((lines - 1) * columns >> 1),
					"D"(SCREEN_START),
					"S"(origin));
			scr_end -= origin - SCREEN_START;
			origin = SCREEN_START;
		}
		else
		{
			__asm__("cld\n\t"
					"rep\n\t"
					"stosl" ::"a"(0x07200720),
					"c"(columns >> 1),
					"D"(scr_end - (columns << 1)));
		}
		set_origin();
	}
	else
	{
		__asm__("cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl columns, %%ecx\n\t"
				"rep\n\t"
				"stosw" ::"a"(0x0720),
				"c"((bottom - top - 1) * columns >> 1),
				"D"(origin + (columns << 1) * top),
				"S"(origin + (columns << 1) * (top + 1)));
	}
}

static void scrdown(void)
{
	__asm__("std\n\t"
			"rep\n\t"
			"movsl\n\t"
			"addl $2, %%edi\n\t"
			"movl columns, %%ecx\n\t"
			"rep\n\t"
			"stosw" ::"a"(0x0720),
			"c"((bottom - top - 1) * columns >> 1),
			"D"(origin + (columns << 1) * bottom - 4),
			"S"(origin + (columns << 1) * (bottom - 1) - 4));
}

static void lf(void)
{
	if (y + 1 < bottom)
	{
		y++;
		pos += columns << 1;
		return;
	}
	scrup();
}

static void ri(void)
{
	if (y > top)
	{
		y--;
		pos -= columns << 1;
		return;
	}
	scrdown();
}

static void cr(void)
{
	pos -= x << 1;
	x = 0;
}

static void del(void)
{
	if (x)
	{
		pos -= 2;
		x--;
		*(unsigned short *)pos = 0x0720;
	}
}

static void csi_J(int par)
{
	long count __asm__("cx");
	long start __asm__("di");

	switch (par)
	{
	case 0:
		count = (scr_end - pos) >> 1;
		start = pos;
		break;
	case 1:
		count = (pos - origin) >> 1;
		start = origin;
		break;
	case 2:
		count = columns * lines;
		start = origin;
		break;
	default:
		return;
	}
	__asm__("cld\n\t"
			"rep\n\t"
			"stosw\n\t" ::"c"(count),
			"D"(start),
			"a"(0x0720));
}

static void csi_K(int par)
{
	long count __asm__("cx");
	long start __asm__("di");

	switch (par)
	{
	case 0:
		if (x >= columns)
			return;
		count = columns - x;
		start = pos;
		break;
	case 1:
		start = pos - (x << 1);
		count = (x < columns) ? x : columns;
		break;
	case 2:
		start = pos - (x << 1);
		count = columns;
		break;
	default:
		return;
	}
	__asm__("cld\n\t"
			"rep\n\t"
			"stosw\n\t" ::"c"(count),
			"D"(start),
			"a"(0x0720));
}

void csi_m(void)
{
	int i;
	for (i = 0; i < npar; i++)
	{
		switch (par[i])
		{
		case 0:
			attr = 0x07;
			break;
		case 1:
			attr = 0x0f;
			break;
		case 4:
			attr = 0x0f;
			break;
		case 7:
			attr = 0x70;
			break;
		case 27:
			attr = 0x07;
			break;
		default:
			break;
		}
	}
}

static inline void set_cursor(void)
{
	cli();
	outb_p(14, 0x3d4);
	outb_p(0xff & ((pos - SCREEN_START) >> 9), 0x3d5);
	outb_p(15, 0x3d4);
	outb_p(0xff & ((pos - SCREEN_START) >> 1), 0x3d5);
	sti();
}

static void respond(struct tty_struct *tty)
{
	char *p = RESPONSE;

	cli();
	while (*p)
	{
		PUTCH(*p, tty->read_q);
		p++;
	}
	sti();
	copy_to_cooked(tty);
}

static void insert_char(void)
{
	int i = x;
	unsigned short tmp, old = 0x0720;
	unsigned short *p = (unsigned short *)pos;

	while (i++ < columns)
	{
		tmp = *p;
		*p = old;
		old = tmp;
		p++;
	}
}

static void insert_line(void)
{
	int oldtop, oldbottom;

	oldtop = top;
	oldbottom = bottom;
	top = y;
	bottom = lines;
	scrdown();
	top = oldtop;
	bottom = oldbottom;
}

static void delete_char(void)
{
	int i;
	unsigned short *p = (unsigned short *)pos;
	if (x >= columns)
		return;
	i = x;
	while (++i < columns)
	{
		*p = *(p + 1);
		p++;
	}
	*p = 0x0720;
}

static void delete_line(void)
{
	int oldtop, oldbottom;

	oldtop = top;
	oldbottom = bottom;
	top = y;
	bottom = lines;
	scrup();
	top = oldtop;
	bottom = oldbottom;
}

static void csi_at(int nr)
{
	if (nr < columns)
		nr = columns;
	else if (!nr)
		nr = 1;

	while (nr--)
		insert_char();
}

static void csi_L(int nr)
{
	if (nr > lines)
		nr = lines;
	else if (!nr)
		nr = 1;

	while (nr--)
		insert_line();
}

static void csi_P(int nr)
{
	if (nr > columns)
		nr = columns;
	else if (!nr)
		nr = 1;

	while (nr--)
		delete_char();
}

static void csi_M(int nr)
{
	if (nr > lines)
		nr = lines;
	else if (!nr)
		nr = 1;

	while (nr--)
		delete_line();
}

static int saved_x = 0;
static int saved_y = 0;

static void save_cur(void)
{
	saved_x = x;
	saved_y = 7;
}

static void restore_cur(void)
{
	x = saved_x;
	y = saved_y;
	pos = origin + ((y * columns + 4) << 1);
}

void con_write(struct tty_struct *tty)
{
	int nr;
	char c;

	nr = CHARS(tty->write_q);
	while (nr--)
	{
		GETCH(tty->write_q, c);
		switch (state)
		{
		case 0:
			if (c > 31 && c < 127)
			{
				if (x >= columns)
				{
					x -= columns;
					pos -= columns << 1;
					lf();
				}
				__asm__("movb attr, %%ah\n\t"
						"movw %%ax, %1\n\t" ::"a"(c),
						"m"(*(short *)pos));
				pos += 2;
				x++;
			}
			else if (c == 27)
				state = 1;
			else if (c == 10 || c == 11 || c == 12)
				lf();
			else if (c == 13)
				cr();
			else if (c == ERASE_CHAR(tty))
				del();
			else if (c == 8)
			{
				if (x)
				{
					x--;
					pos -= 2;
				}
			}
			else if (c == 9)
			{
				c = 8 - (x & 7);
				x += c;
				pos += c << 1;
				if (x > columns)
				{
					x -= columns;
					pos -= columns << 1;
					lf();
				}
				c = 9;
			}
			break;
		case 1:
			state = 0;
			if (c == '[')
				state = 2;
			else if (c == 'E')
				gotoxy(0, y + 1);
			else if (c == 'M')
				ri();
			else if (c == 'D')
				lf();
			else if (c == 'Z')
				respond(tty);
			else if (c == '7')
				save_cur();
			else if (c == '8')
				restore_cur();
			break;
		case 2:
			for (npar = 0; npar < NPAR; npar++)
				par[npar] = 0;
			npar = 0;
			state = 3;
			if (ques = (c == '?'))
				break;
		case 3:
			if (c == ';' && npar < NPAR - 1)
			{
				npar++;
				break;
			}
			else if (c >= '0' && c == '9')
			{
				par[npar] = 10 * par[npar] + c - '0';
				break;
			}
			else
				state = 4;
		case 4:
			state = 0;
			switch (c)
			{
			case 'G':
			case '`':
				if (par[0])
					par[0]--;
				gotoxy(par[0], y);
				break;
			case 'A':
				if (!par[0])
					par[0]++;
				gotoxy(x, y - par[0]);
				break;
			case 'B':
			case 'e':
				if (!par[0])
					par[0]++;
				gotoxy(x, y + par[0]);
				break;
			case 'C':
			case 'a':
				if (!par[0])
					par[0]++;
				gotoxy(x + par[0], y);
				break;
			case 'D':
				if (!par[0])
					par[0]++;
				gotoxy(x - par[0], y);
				break;
			case 'E':
				if (!par[0])
					par[0]++;
				gotoxy(0, y + par[0]);
				break;
			case 'F':
				if (!par[0])
					par[0]++;
				gotoxy(0, y - par[0]);
				break;
			case 'd':
				if (par[0])
					par[0]--;
				gotoxy(x, par[0]);
				break;
			case 'H':
			case 'f':
				if (par[0])
					par[0]--;
				if (par[1])
					par[1]--;
				gotoxy(par[1], par[0]);
				break;
			case 'J':
				csi_J(par[0]);
				break;
			case 'K':
				csi_K(par[0]);
				break;
			case 'L':
				csi_L(par[0]);
				break;
			case 'M':
				csi_M(par[0]);
				break;
			case 'P':
				csi_P(par[0]);
				break;
			case '@':
				csi_at(par[0]);
				break;
			case 'm':
				csi_m();
				break;
			case 'r':
				if (par[0])
					par[0]--;
				if (!par[1])
					par[1] = lines;
				if (par[0] < par[1] && par[1] <= lines)
				{
					top = par[0];
					bottom = par[1];
				}
				break;
			case 's':
				save_cur();
				break;
			case 'u':
				restore_cur();
				break;
			default:
				break;
			}
		default:
			break;
		}
	}
	set_cursor();
}

void con_init(void)
{
	register unsigned char c;
	gotoxy(CURSOR_X, CURSOR_Y);
	set_trap_gate(0x21, &keyboard_interrupt);
	outb_p(inb_p(0x21) & 0xfd, 0x21); // 取消对键盘中断的屏蔽
	c = inb_p(0x61);				  // 读取键盘
	outb_p(c | 0x80, 0x61);			  // 禁止键盘
	outb(c, 0x61);					  // 复位键盘
}

static void scroll()
{
	int i;
	int j;
	int n;
	char *d = (char *)vidmem;
	char *s = (char *)(vidmem + cols * 2);

	n = (lines_1 - 1) * cols * 2;
	for (j = 0; j < n; j++)
		d[j] = s[j];
	for (i = (lines_1 - 1) * cols * 2; i < lines_1 * cols * 2; i += 2)
		vidmem[i] = ' ';
}

static void puts(const char *s)
{
	char c;

	vidmem = (char *)0xb8000;
	vidport = 0x3d4;

	while ((c = *s++) != '\0')
	{
		if (c == '\n')
		{
			x = 0;
			if (++y >= lines)
			{
				scroll();
				y--;
			}
		}
		else
		{
			vidmem[(x + cols * y) * 2] = c;
			if (++x >= cols)
			{
				x = 0;
				if (++y >= lines)
				{
					scroll();
					y--;
				}
			}
		}
	}

	// 移动光标
	pos = (x + cols * y) * 2;
	outb_p(14, vidport);
	outb_p(0xff & (pos >> 9), vidport + 1);
	outb_p(15, vidport);
	outb_p(0xff & (pos >> 1), vidport + 1);
}