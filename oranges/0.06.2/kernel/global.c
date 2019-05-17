#include <const.h>
#include <type.h>
#include <proto.h>

#define DEF_GLOBAL
#include <global.h>

struct Task task_table[NR_TASKS] = {
	{ TestA, STACK_SIZE_TESTA, "TestA" },
	{ TestB, STACK_SIZE_TESTB, "TestB" },
	{ TestC, STACK_SIZE_TESTC, "TestC" },
	{ task_tty, STACK_SIZE_TTY, "tty" }
};

system_call syscall_table[NR_SYSCALL]  = {
	sys_get_ticks
};