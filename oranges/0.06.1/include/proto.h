#ifndef _PROTO_H
#define _PROTO_H

// kliba.s
void out_byte(unsigned short port, unsigned char value);
unsigned char in_byte(unsigned short port);
void disp_str(char* s);
void disp_color_str(char* s, int color);
void* memcpy(void* dest, void* src, int size);
void memset(void* p_dst, char ch, int size);
char* strcpy(char* p_dst, char* p_src);
void disable_irq(int irq);
void enable_irq(int irq);
void disable_int();
void enable_int();

// kernel.s
void restart();
void sys_call();

// syscall.s
int get_ticks();

// klib.c
void disp_int(int n);

// tty.c
void task_tty();
void in_process(unsigned int key);

// keyboard.c
void init_keyboard();
void keyboard_read();

// i8259.c
void init_8259A();
void spurious_irq(int irq);
void put_irq_handler(int irq, irq_handler handler);

// protect.c
void init_protect();
unsigned int seg2phys(unsigned short seg);
int sys_get_ticks();
void schedule();

// clock.c
void clock_handler(int irq);
void milli_delay(int ms);
void init_clock();

// main.c
void TestA();
void TestB();
void TestC();

#endif 