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

void delay(int time)
{
	int i,j,k;
	for (k=0; k<time; k++)
		for (i=0; i<100; i++)
			for (j=0; j < 10000; j++) {
				// do nothing
			}
}