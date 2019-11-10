#ifndef __SYS_WAIT_H__
#define __SYS_WAIT_H__

#include <sys/types.h>

#define _LOW(v)		( (v) & 0377 )
#define _HIGH(v)	( ((v) >> 8) & 0377 )

#define WNOHANG		1
#define WUNTRACED	2

#define WIFEXITED(s)	( !((s)&0xff) )
#define WIFSTOPPED(s)	( ((s)&0xff) == 0x7f )
#define WEXITSTATUS(s)	( ((s)>>8)&0xff )
#define WTERMSIG(s)		( (s) & 0x7f )
#define WSTOPSIG(s)		( ((s)>>8) & 0xff )
#define WIFSIGNALED(s)	( ((unsigned int)(s) - 1 & 0xffff) < 0xff )

pid_t wait(int *stat_loc);
pid_t waitpid(pid_t pid, int *sta_loc, int options);

#endif //__SYS_WAIT_H__