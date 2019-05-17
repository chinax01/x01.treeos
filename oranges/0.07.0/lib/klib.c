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
	return;
}

void spin(const char* s)
{
	return;
}