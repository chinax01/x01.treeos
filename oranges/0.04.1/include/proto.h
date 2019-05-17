#ifndef _PROTO_H
#define _PROTO_H

void out_byte(unsigned short port, unsigned char value);
unsigned char in_byte(unsigned short port);
void disp_str(char* s);
void disp_color_str(char* s, int color);
void* memcpy(void* dest, void* src, int size);

void disp_int(int n);
void init_8259A();
void init_protect();
void spurious_irq(int irq);

#endif 