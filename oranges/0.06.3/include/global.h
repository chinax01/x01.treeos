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
EXTERN int ticks;

EXTERN unsigned char gdt_ptr[6];
EXTERN struct Descriptor gdt[GDT_SIZE];
EXTERN unsigned char idt_ptr[6];
EXTERN struct Gate idt[IDT_SIZE];

EXTERN struct TSS tss;
EXTERN struct Process* proc_ready;
EXTERN struct Process proc_table[NR_TASKS + NR_PROCS];

EXTERN char task_stack[STACK_SIZE_TOTAL];
EXTERN irq_handler irq_table[NR_IRQ];

EXTERN int k_reenter;

EXTERN struct TTY tty_table[NR_CONSOLES];
EXTERN struct Console console_table[NR_CONSOLES];
EXTERN int nr_current_console;

extern struct Task task_table[];
extern struct Task user_proc_table[];
extern system_call syscall_table[];

#endif //_GLOBAL_H