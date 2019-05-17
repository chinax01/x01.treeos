#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int printf(const char* fmt, ...)
{
	int i = 0;
	char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4);
	i = vsprintf(buf, fmt, arg);
	write(buf, i);

	return i;
}

int vsprintf(char* buf, const char* fmt, va_list args)
{
	char* p;
	char tmp[256];
	va_list next = args;

	for (p = buf; *fmt; fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt++;
			continue;
		}
		fmt++;
		switch (*fmt) {
		case 'x':
			itoa(tmp, *((int*)next));
			strcpy(p, tmp);
			next += 4;
			p += strlen(tmp);
			break;
		case 's':
			break;
		default:
			break;
		}
	}

	return (p-buf);
}