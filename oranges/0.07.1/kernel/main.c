#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	struct Task* p_task;
	struct Process* p_proc= proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16   selector_ldt = SELECTOR_LDT_FIRST;
        u8    privilege;
        u8    rpl;
	int   eflags;
	int   i;
	int   prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
	        if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio      = 15;
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
			prio      = 5;
                }

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid = i;			/* pid */

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(struct Descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(struct Descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		p_proc->nr_tty		= 0;

		p_proc->flags = 0;
		p_proc->message = 0;
		p_proc->recvfrom = NO_TASK;
		p_proc->sendto = NO_TASK;
		p_proc->has_int_message = 0;
		p_proc->sending = 0;
		p_proc->next = 0;

		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

        proc_table[NR_TASKS + 0].nr_tty = 0;
        proc_table[NR_TASKS + 1].nr_tty = 1;
        proc_table[NR_TASKS + 2].nr_tty = 1;

	k_reenter = 0;
	ticks = 0;

	proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}

int kernel_main2()
{
	struct Task *t = task_table;
	struct Process *p = proc_table;
	char *stack = task_stack + STACK_SIZE_TOTAL;
	unsigned short selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	unsigned char privilege;
	unsigned char rpl;
	int eflags;
	int priority;
	
	for (i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		if (i < NR_TASKS) {
			t = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
			priority = 15;
		} else {
			t = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
			priority = 5;
		}

		strcpy(p->name, t->name);
		p->pid = i;
		p->ldt_sel = selector_ldt;
		
		memcpy(&p->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(struct Descriptor));
		p->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(struct Descriptor));
		p->ldts[1].attr1 = DA_DRW | privilege << 5;

		p->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip = (unsigned int)t->initial_eip;
		p->regs.esp = (unsigned int)stack;
		p->regs.eflags = eflags;
		
		p->nr_tty = 0;
		p->ticks = p->priority = priority;

		p->flags = 0;
		p->message = 0;
		p->recvfrom = NO_TASK;
		p->sendto = NO_TASK;
		p->has_int_message = 0;
		p->sending = 0;
		p->next = 0;

		stack -= t->stacksize;
		p++;
		t++;
		selector_ldt += 1 << 3;
	}

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

int get_ticks()
{
	struct Message m;
	reset_msg(&m);
	m.type = GET_TICKS;
	send_rec(BOTH, TASK_SYS, &m);
	
	return m.RETVAL;
}

void TestA()
{
	while (1)
	{
		printf("<Ticks:%x>", get_ticks());
		milli_delay(200);
	}
}

void TestB()
{
	while (1)
	{
		printf("B");
		milli_delay(200);
	}
}

void TestC()
{
	while (1)
	{
		printf("C");
		milli_delay(200);
	}
}

