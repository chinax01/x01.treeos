#ifndef _PROTO_H
#define _PROTO_H

#include <fs.h>

// kliba.s
void out_byte(unsigned short port, unsigned char value);
unsigned char in_byte(unsigned short port);
void disp_str(char* s);
void disp_color_str(char* s, int color);
void disable_irq(int irq);
void enable_irq(int irq);
void disable_int();
void enable_int();
void port_read(u16 port, void* buf, int n);
void port_write(u16 port, void* buf, int n);

#define phys_copy	memcpy 
#define phys_set	memset 

// kernel.s
void restart();
void sys_call();

// syscall.s
int sendrec(int func, int src_dest, struct Message* m);
void printx(char* s);

// klib.c
void disp_int(int n);
void panic(const char* fmt, ...);
void get_boot_params(struct boot_params* bp);
int get_kernel_map(unsigned int *base, unsigned int* limit);

// lib/
int open(const char* pathname, int flags);
int close(int fd);
int read(int fd, void* buf, int count);
int write(int fd, const void* buf, int count);
int unlink(const char* pathname);
int getpid();
int wait(int *status);
void exit(int status);
int fork();
int send_rec(int func, int src_dest, struct Message *m);
void spin(const char* s);
void assertion_failure(char* exp, char* file, char* base_file, int line);
int memcmp(const void * s1, const void *s2, int n);
int strcmp(const char* s1, const char* s2);
char* strcat(char* s1, const char* s2);
void* memcpy(void* dest, void* src, int size);
void memset(void* p_dst, char ch, int size);
char* strcpy(char* p_dst, char* p_src);
int strlen(const char* p_str);
int execl(const char* path, const char* arg, ...);
int execv(const char* path, char* argv[]);

// mm
void task_mm();
int do_fork();
int alloc_mem(int pid, int memsize);
void do_wait();
void do_exit(int status);
int free_mem(int pid);
int do_exec();

// fs/main.c
void task_fs();
int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf);
struct inode* get_inode(int dev, int num);
struct super_block* get_super_block(int dev);
void sync_inode(struct inode* node);
void put_inode(struct inode* node);

// fs/
int do_open();
int do_rdwt();
int do_unlink();
int do_close();
int do_stat();

// fs/misc.c
int search_file(char* path);
int strip_path(char* finename, const char* pathname, struct inode** ppinode);

// hd.c
void task_hd();
void hd_handler(int irq);

// systask.c
void task_sys();

// proc.c
int sys_sendrec(int func, int src_dest, struct Message* m, struct Process* p);
void* va2la(int pid, void* va);
int ldt_seg_linear(struct Process* p, int index);
void reset_msg(struct Message* m);
void dump_msg(const char * title, struct Message* m);
void inform_int(int task_nr);

// printf.c
int printf(const char* fmt, ...);
int printl(const char* fmt, ...);
int vsprintf(char* buf, const char* fmt, va_list args);
int sprintf(char* buf, const char *fmt, ...);
char* i2a(int val, int base, char** ps);

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
//int sys_write(char* buf, int len, int edx, struct Process* p);
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
unsigned int seg2linear(unsigned short seg);
int sys_get_ticks();
void schedule();
void init_idt_desc(unsigned char vector, unsigned char desc_type, int_handler handler, unsigned char privilege);
void init_descriptor(struct Descriptor *desc,
					unsigned int base, unsigned int limit, unsigned short attribute);
// clock.c
void clock_handler(int irq);
void milli_delay(int ms);
void init_clock();

// main.c
void Init();
void TestA();
void TestB();
void TestC();
void clear();
int get_ticks();
void untar(const char * filename);

#endif 