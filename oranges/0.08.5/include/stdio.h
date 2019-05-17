#ifndef _STDIO_H	
#define _STDIO_H

#define O_CREAT		1
#define O_RDWR		2

#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#define MAX_PATH	128

int open(const char* pathname, int flags);
int close(int fd);

#endif //_STDIO_H