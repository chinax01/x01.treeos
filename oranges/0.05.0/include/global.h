#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <const.h>
#include <type.h>

#ifdef DEF_GLOBAL
	#define EXTERN	
#else 
	#define EXTERN  extern 
#endif 

EXTERN int disp_pos;

EXTERN unsigned char gdt_ptr[6];
EXTERN struct Descriptor gdt[GDT_SIZE];
EXTERN unsigned char idt_ptr[6];
EXTERN struct Gate idt[IDT_SIZE];

EXTERN struct TSS tss;
EXTERN struct Process* proc_ready;

EXTERN struct Process proc_table[NR_TASKS];
EXTERN char task_stack[STACK_SIZE_TOTAL];

#endif //_GLOBAL_H