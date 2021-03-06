#include <const.h>
#include <type.h>
#include <proto.h>
#include <hd.h>
#include <fs.h>

#define DEF_GLOBAL
#include <global.h>

struct Task task_table[NR_TASKS] = {
	{ task_tty, STACK_SIZE_TTY, "tty" },
	{ task_sys, STACK_SIZE_SYS, "sys" },
	{ task_hd, STACK_SIZE_HD, "hd" },
	{ task_fs, STACK_SIZE_FS, "fs" },
	{ task_mm, STACK_SIZE_MM, "mm"}
};
struct Task user_proc_table[NR_NATIVE_PROCS] = {
	{ Init, STACK_SIZE_INIT, "INIT" },
	{ TestA, STACK_SIZE_TESTA, "TestA" },
	{ TestB, STACK_SIZE_TESTB, "TestB" },
	{ TestC, STACK_SIZE_TESTC, "TestC" }
};

system_call syscall_table[NR_SYSCALL]  = {
	sys_sendrec,
	sys_printx
};

// fs
u8* fsbuf = (u8*)0x600000;
const int FSBUF_SIZE = 0x100000;

struct dev_drv_map dd_map[] = {
	{INVALID_DRIVER},
	{INVALID_DRIVER},
	{INVALID_DRIVER},
	{TASK_HD},
	{TASK_TTY},
	{INVALID_DRIVER}
};

// mm
u8* mmbuf = (u8*)0x700000;
const int MMBUF_SIZE = 0x100000;
