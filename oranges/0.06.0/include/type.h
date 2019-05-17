#ifndef _TYPE_H
#define _TYPE_H

#include <const.h>

typedef void (*int_handler)();
typedef void (*task_f)();
typedef void (*irq_handler)(int irq);
typedef void* system_call;

struct Descriptor {
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_mid;
	unsigned char attr1;
	unsigned char limit_high_attr2;
	unsigned char base_high;
};

struct Gate {
	unsigned short offset_low;
	unsigned short selector;
	unsigned char dcount;
	unsigned char attr;
	unsigned short offset_high;
};



struct StackFrame {
	unsigned int gs;
	unsigned int fs;
	unsigned int es;
	unsigned int ds;
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int kernel_esp;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;
	unsigned int retaddr;
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int ss;
};

struct Process {
	struct StackFrame regs;
	unsigned short ldt_sel;
	struct Descriptor ldts[LDT_SIZE];
	
	int ticks;
	int priority;

	unsigned pid;
	char p_name[16];
};

struct TSS {
	unsigned int backlink;
	unsigned int esp0;
	unsigned int ss0;
	unsigned int esp1;
	unsigned int ss1;
	unsigned int esp2;
	unsigned int ss2;
	unsigned int cr3;
	unsigned int eip;
	unsigned int flags;
	unsigned int eax;
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned int es; 
	unsigned int cs;
	unsigned int ss;
	unsigned int ds; 
	unsigned int fs; 
	unsigned int gs; 
	unsigned int ldt;
	unsigned short trap;
	unsigned short iobase;
};

struct Task {
	task_f	initial_eip;
	int stacksize;
	char name[32];
};

#endif 