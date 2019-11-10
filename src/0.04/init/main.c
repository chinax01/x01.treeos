#define __LIBRARY__
#include <unistd.h>
#include <time.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <asm/system.h>
#include <asm/io.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/fs.h>

_syscall0(int, fork)
_syscall0(int, pause)
_syscall0(int, setup)
_syscall0(int, sync)

static inline long fork_for_process0() {
	long __res;
	__asm__ volatile (
		"int $0x80\n\t"  														/* 调用系统中断0x80 */
		: "=a" (__res)  														/* 返回值->eax(__res) */
		: "0" (2));  															/* 输入为系统中断调用号__NR_name */
	if (__res >= 0)  															/* 如果返回值>=0,则直接返回该值 */
		return __res;
	errno = -__res;  															/* 否则置出错号,并返回-1 */
	return -1;
}

#define CURSOR_X	(*(unsigned char*)0x90000)
#define CURSOR_Y	(*(unsigned char*)0x90001)

static char printbuf[1024];
static char *testbuf = "1234567890";

extern int vsprintf();
extern void init(void);
extern void hd_init(void);
extern void tty_init();
extern void trap_init();
extern long kernel_mktime(struct tm* tm);
extern int chdir(const char* filename);
extern long startup_time;

static void time_init();
void printa(void);
void print_char(char c, int x, int y);


#define CMOS_READ(addr)	({ \
outb_p(0x80|(addr), 0x70); \
inb_p(0x71); })

#define BCD_TO_BIN(v)	((v)=((v)&15) + ((v)>>4)*10)

static void time_init()
{
	struct tm time;
	do {
		time.tm_sec = CMOS_READ(0);
		time.tm_min = CMOS_READ(2);
		time.tm_hour = CMOS_READ(4);
		time.tm_mday = CMOS_READ(7);
		time.tm_mon = CMOS_READ(8)-1;
		time.tm_year = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	startup_time = kernel_mktime(&time);
}

void main(void)
{
	time_init();
	tty_init();
	trap_init();
	sched_init();
	buffer_init();
	hd_init();
	sti();
	move_to_user_mode();

	if (!fork_for_process0()) init();

	for (;;) pause();
}

static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1, printbuf, i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

void printa(void)
{
	print_char('a', 0, 0);
}

void print_char(char c, int x, int y)
{
	char* video = (char*)0xb8000;
	video[(y*80+x) * 2] = c;
}

static char* argv[] = { "-", NULL };
static char* envp[] = { "HOME=/usr/root", NULL };

void init(void)
{
	int i, j;
	setup();
	MP("");
}