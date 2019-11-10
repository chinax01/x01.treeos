#ifndef __UTIME_H__
#define __UTIME_H__

#include <sys/types.h>

struct utimbuf {
	time_t actime;
	time_t modtime;
};

extern int utime(const char *filename, struct utimbuf *times);


#endif //__UTIME_H__