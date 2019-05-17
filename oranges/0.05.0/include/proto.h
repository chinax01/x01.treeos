#ifndef _PROTO_H
#define _PROTO_H

// kliba.s
void out_byte(unsigned short port, unsigned char value);
unsigned char in_byte(unsigned short port);
void disp_str(char* s);
void disp_color_str(char* s, int color);
void* memcpy(void* dest, void* src, int size);
void memset(void* p_dst, char ch, int size);

// kernel.s
void restart();

// klib.c
void disp_int(int n);
void delay(int time);

// i8259.c
void init_8259A();
void spurious_irq(int irq);

// protect.c
void init_protect();
unsigned int seg2phys(unsigned short seg);

// main.c
void TestA();

#endif 