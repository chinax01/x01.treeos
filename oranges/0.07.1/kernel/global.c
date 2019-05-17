#include <const.h>
#include <type.h>
#include <proto.h>

#define DEF_GLOBAL
#include <global.h>

struct Task task_table[NR_TASKS] = {
	{ task_tty, STACK_SIZE_TTY, "tty" },
	{ task_sys, STACK_SIZE_SYS, "sys" }
};
struct Task user_proc_table[NR_PROCS] = {
	{ TestA, STACK_SIZE_TESTA, "TestA" },
	{ TestB, STACK_SIZE_TESTB, "TestB" },
	{ TestC, STACK_SIZE_TESTC, "TestC" }
};

system_call syscall_table[NR_SYSCALL]  = {
	sys_sendrec,
	sys_printx
};