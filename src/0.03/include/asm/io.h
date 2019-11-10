#ifndef __IO_H__
#define __IO_H__

#define outb(v,p)	\
__asm__("outb %%al, %%dx"::"a"(v),"d"(p))

#define inb(p)	({\
unsigned char _v; \
__asm__ volatile("inb %%dx, %%al":"=a"(_v):"d"(p)); \
_v; })

#define outb_p(v,p)	\
__asm__("outb %%al, %%dx\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:"::"a"(v),"d"(p))

#define inb_p(p)	({ \
unsigned char _v; \
__asm__ volatile("inb %%dx, %%al\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:" : "=a"(_v) : "d"(p)); \
_v; })

#define __INS(s)	\
static inline void ins##s(unsigned short port, void* addr, unsigned long count) \
{ __asm__ __volatile__ ("cld; rep; ins" #s \
: "=D"(addr),"=c"(count) : "d"(port),"0"(addr),"1"(count)); }

__INS(w)

#endif //__IO_H__