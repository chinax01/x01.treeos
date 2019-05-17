#ifndef _TYPE_H
#define _TYPE_H

#include <const.h>

typedef void (*int_handler)();
typedef void (*task_f)();
typedef void (*irq_handler)(int irq);
typedef void* system_call;
typedef char* va_list;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

struct Descriptor {
	u16 limit_low;
	u16 base_low;
	u8 base_mid;
	u8 attr1;
	u8 limit_high_attr2;
	u8 base_high;
};

struct Gate {
	u16 offset_low;
	u16 selector;
	u8 dcount;
	u8 attr;
	u16 offset_high;
};

struct StackFrame {
	u32 gs;
	u32 fs;
	u32 es;
	u32 ds;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 kernel_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	u32 retaddr;
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;
	u32 ss;
};

struct Process {
	struct StackFrame regs;
	u16 ldt_sel;
	struct Descriptor ldts[LDT_SIZE];
	
	int ticks;
	int priority;

	u32 pid;
	char name[16];
	int flags;
	
	struct Message* message;
	int recvfrom;
	int sendto;
	int has_int_message;
	
	struct Process* sending;
	struct Process* next;
	
	int nr_tty;
};

struct TSS {
	u32 backlink;
	u32 esp0;
	u32 ss0;
	u32 esp1;
	u32 ss1;
	u32 esp2;
	u32 ss2;
	u32 cr3;
	u32 eip;
	u32 flags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 es; 
	u32 cs;
	u32 ss;
	u32 ds; 
	u32 fs; 
	u32 gs; 
	u32 ldt;
	u16 trap;
	u16 iobase;
};

struct Task {
	task_f	initial_eip;
	int stacksize;
	char name[32];
};

struct Console {
	u32 current_start_addr;
	u32 original_addr;
	u32 v_mem_limit;
	u32 cursor;
};

struct TTY {
	u32 in_buf[TTY_IN_BYTES];
	u32* p_inbuf_head;
	u32* p_inbuf_tail;
	int inbuf_count;

	struct Console* p_console;
};

struct mess1 {
	int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};

struct mess2 {
	void* m2p1;
	void* m2p2;
	void* m2p3;
	void* m2p4;
};

struct mess3 {
	int m3i1;
	int m3i2;
	int m3i3;
	int m3i4;
	u64 m3l1;
	u64 m3l2;
	void* m3p1;
	void* m3p2;
};

struct Message {
	int source;
	int type;
	union {
		struct mess1 m1;
		struct mess2 m2;
		struct mess3 m3;
	} u;
};


#endif 