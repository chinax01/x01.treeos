#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

char* itoa(char* s, int n)
{
	char* p = s;
	char c;
	int i;
	int flag = 0;

	*p++ = '0';
	*p++ = 'x';
	if (n == 0) {
		*p++ = '0';
	} else {
		for (i = 28; i >= 0; i -= 4) {
			c = (n >> i) & 0xF;
			if (flag || (c > 0)) {
				flag = 1;
				c += '0';
				if (c > '9') {
					c += 7;
				}
				*p++ = c;
			}
		}
	}
	*p = 0;
	
	return s;
}

void disp_int(int n)
{
	char buf[16];
	itoa(buf, n);
	disp_str(buf);
}

void assertion_failure(char* exp, char* file, char* base_file, int line)
{
	printl("%c  assert(%s) failed, file: %s, base_file: %s, line: %d",
		MAG_CH_ASSERT, exp, file, base_file, line);
	spin("assertion_failure()");
	__asm__ __volatile__("ud2");
}

void panic(const char* fmt, ...)
{
	int i;
	char buf[256];

	va_list arg = (va_list)((char*)&fmt + 4);
	i = vsprintf(buf, fmt, arg);

	printl("%c panic! %s", MAG_CH_PANIC, buf);

	__asm__ __volatile__("ud2");
}

void spin(const char* func_name)
{
	printl("\nspinning in %s ...\n", func_name);
	while(1) { }
}

int memcmp(const void * s1, const void *s2, int n)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = (const char *)s1;
	const char * p2 = (const char *)s2;
	int i;
	for (i = 0; i < n; i++,p1++,p2++) {
		if (*p1 != *p2) {
			return (*p1 - *p2);
		}
	}
	return 0;
}