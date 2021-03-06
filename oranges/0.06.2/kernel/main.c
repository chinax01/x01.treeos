#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int kernel_main()
{
	disp_str("------------------Kernel main------------------\n");

	struct Task *t = task_table;
	struct Process *p = proc_table;
	char *stack = task_stack + STACK_SIZE_TOTAL;
	unsigned short selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	
	for (i = 0; i < NR_TASKS; i++)
	{
		strcpy(p->p_name, t->name);
		p->pid = i;
		p->ldt_sel = selector_ldt;
		
		memcpy(&p->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(struct Descriptor));
		p->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(struct Descriptor));
		p->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

		p->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;
		p->regs.eip = (unsigned int)t->initial_eip;
		p->regs.esp = (unsigned int)stack;
		p->regs.eflags = 0x1202;

		stack -= t->stacksize;
		p++;
		t++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority = 5;
	proc_table[2].ticks = proc_table[2].priority = 3;
	proc_table[3].ticks = proc_table[3].priority = 10;

	k_reenter = 0;
	ticks = 0;

	proc_ready = proc_table;

	clear();

	init_clock();
	init_keyboard();

	restart();

	while (1)
	{
	}
}

void clear()
{
	int i;
	disp_pos = 0;
	for (i = 0; i < 80*25; i++) {
		disp_str(" ");
	}
	disp_pos = 0;
}

void TestA()
{
	while (1)
	{
		//disp_color_str("A", BRIGHT | MAKE_COLOR(BLACK, RED));
		milli_delay(300);
	}
}

void TestB()
{
	while (1)
	{
		//disp_color_str("B", BRIGHT | MAKE_COLOR(BLACK, BLUE));
		milli_delay(900);
	}
}

void TestC()
{
	while (1)
	{
		//disp_str("C");
		milli_delay(1500);
	}
}

