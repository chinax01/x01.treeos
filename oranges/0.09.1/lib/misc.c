#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int send_rec(int func, int src_dest, struct Message *m)
{
	int ret = 0;

	if (func == RECEIVE)
		memset(m, 0, sizeof(struct Message));

	switch (func)
	{
	case BOTH:
		ret = sendrec(SEND, src_dest, m);
		if (ret == 0)
			ret = sendrec(RECEIVE, src_dest, m);
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(func, src_dest, m);
	default:
		assert(func == BOTH || func == SEND || func == RECEIVE);
		break;
	}

	return ret;
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

int strcmp(const char* s1, const char* s2)
{
	if (s1 == 0 || s2 == 0) return s1 - s2;
	const char* p1 = s1;
	const char* p2 = s2;
	for (; *p1 && *p2; p1++, p2++) {
		if (*p1 != *p2) break;
	}
	return *p1 - *p2;
}

char* strcat(char* s1, const char* s2)
{
	if (s1 == 0 || s2 == 0) return 0;

	char* p1 = s1;
	for(; *p1; p1++) { }

	const char* p2 = s2;
	for(; *p2; p1++, p2++) {
		*p1 = *p2;
	}
	*p1 = 0;

	return s1;
}

void spin(const char* func_name)
{
	printl("\nspinning in %s ...\n", func_name);
	while(1) { }
}

void assertion_failure(char* exp, char* file, char* base_file, int line)
{
	printl("%c  assert(%s) failed, file: %s, base_file: %s, line: %d",
		MAG_CH_ASSERT, exp, file, base_file, line);
	spin("assertion_failure()");
	__asm__ __volatile__("ud2");
}