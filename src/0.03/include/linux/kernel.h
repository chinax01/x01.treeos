#ifndef __KERNEL_H__
#define __KERNEL_H__

void verify_area(void* addr, int count);
void panic(const char* str);
int printf(const char* fmt, ...);
int printk(const char* fmt, ...);
int tty_write(unsigned ch, char* buf, int count);

#endif //__KERNEL_H__