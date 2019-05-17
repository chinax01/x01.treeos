#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

int kernel_main()
{
	disp_str("--------------kernel_main begin----------\n");
	
	struct Process* p = proc_table;
	p->ldt_sel = SELECTOR_LDT_FIRST;
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
	p->regs.eip = (unsigned int)TestA;
	p->regs.esp = (unsigned int)task_stack + STACK_SIZE_TOTAL;
	p->regs.eflags = 0x1202;

	proc_ready = proc_table;
	restart();

	while (1) { }
}

void TestA()
{
	int i = 0;
	while (1) {
		disp_str("A");
		disp_int(i++);
		disp_str(".");
		delay(1);
	}
}