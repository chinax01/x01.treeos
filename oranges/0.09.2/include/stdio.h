#ifndef _STDIO_H	
#define _STDIO_H

#include <type.h>

#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

// string
#define STR_DEFAULT_LEN		1024

#define O_CREAT		1
#define O_RDWR		2

#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#define MAX_PATH	128

struct stat {
	int st_dev;
	int st_ino;
	int st_mode;
	int st_rdev;
	int st_size;
};

struct time {
	u32 year;
	u32 month;
	u32 day;
	u32 hour;
	u32 minute;
	u32 second;
};

#define BCD_TO_DEC(x)	( ((x) >> 4) * 10 + ((x) & 0x0F) )

int printf(const char* fmt, ...);
int printl(const char* fmt, ...);
int vsprintf(char* buf, const char* fmt, va_list args);
int sprintf(char* buf, const char* fmt, ...);

int open(const char* pathname, int flags);
int close(int fd);
int read(int fd, void* buf, int count);
int write(int fd, const void* buf, int count);
int unlink(const char* pathname);
int getpid();
int wait(int *status);
void exit(int status);
int fork();
int stat(const char* path, struct stat* buf);

int exec(const char * path);
int execl(const char *path, const char *arg, ...);
int execv(const char *path, char * argv[]);


#endif //_STDIO_H