#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <stdio.h>

int kernel_main()
{
	struct Task *t = task_table;
	struct Process *p = proc_table;
	char *stack = task_stack + STACK_SIZE_TOTAL;
	unsigned short selector_ldt = SELECTOR_LDT_FIRST;
	int i,j;
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

		for (j = 0; j < NR_FILES; j++) {
			p->filp[j] = 0;
		}

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
	int fd;
	int i;
	char* filenames[] = { "/foo", "/bar", "/baz" };
	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]);i++) {
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		printf("file %s created, fd: %d\n", filenames[i], fd);
		close(fd);
	}

	for (i = 0; i < sizeof(filenames) /sizeof(filenames[0]); i++) {
		if (unlink(filenames[i]) == 0)
			printf("File removed %s\n", filenames[i]);
		else 
			printf("Failed to remove file: %s\n", filenames[i]);
	}

	spin("TestA");
}

void TestB()
{
	while (1)
	{
		//printf("B");
		milli_delay(200);
	}
}

void TestC()
{
	while (1)
	{
		//printf("C");
		milli_delay(200);
	}
}

