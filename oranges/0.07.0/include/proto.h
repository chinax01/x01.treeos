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
int strlen(char* p_str);
void disable_irq(int irq);
void enable_irq(int irq);
void disable_int();
void enable_int();

// kernel.s
void restart();
void sys_call();

// syscall.s
int get_ticks();
void write(char* buf, int len);
int sendrec(int func, int src_dest, struct Message* m);
void printx(char* s);

// klib.c
void disp_int(int n);
void panic(const char* fmt, ...);
void spin(const char* s);
//void assertion_failure(char* exp, char* file, char* base_file, int line);

// proc.c
int sys_sendrec(int func, int src_dest, struct Message* m, struct Process* p);
void* va2la(int pid, void* va);
int ldt_seg_linear(struct Process* p, int index);

// printf.c
int printf(const char* fmt, ...);
int vsprintf(char* buf, const char* fmt, va_list args);
#define printl	printf 

// console.c
void init_screen(struct TTY* tty);
void out_char(struct Console *con, char c);
void select_console(int nr);
int is_current_console(struct Console * con);
void scroll_screen(struct Console* con, int direction);

// tty.c
void task_tty();
void in_process(struct TTY* tty, unsigned int key);
void tty_write(struct TTY* t, char* buf, int len);
int sys_write(char* buf, int len, int edx, struct Process* p);
int sys_printx(int ebx, int ecx, char* s, struct Process* proc);

// keyboard.c
void init_keyboard();
void keyboard_read(struct TTY* tty);

// i8259.c
void init_8259A();
void spurious_irq(int irq);
void put_irq_handler(int irq, irq_handler handler);

// protect.c
void init_protect();
unsigned int seg2phys(unsigned short seg);
int sys_get_ticks();
void schedule();
void init_idt_desc(unsigned char vector, unsigned char desc_type, int_handler handler, unsigned char privilege);

// clock.c
void clock_handler(int irq);
void milli_delay(int ms);
void init_clock();

// main.c
void TestA();
void TestB();
void TestC();
void clear();

#endif 