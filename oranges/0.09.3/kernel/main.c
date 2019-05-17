#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <stdio.h>

int kernel_main()
{
	struct Task *t;
	struct Process *p = proc_table;
	char *stack = task_stack + STACK_SIZE_TOTAL;
	int i, j;
	unsigned char privilege;
	unsigned char rpl;
	int eflags;
	int priority;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++, t++)
	{
		if (i >= NR_TASKS + NR_NATIVE_PROCS)
		{
			p->flags = FREE_SLOT;
			continue;
		}

		if (i < NR_TASKS)
		{
			t = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
			priority = 15;
		}
		else
		{
			t = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
			priority = 5;
		}

		strcpy(p->name, t->name);
		p->parent = NO_TASK;

		if (strcmp(t->name, "INIT") != 0)
		{
			p->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
			p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];
			p->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;
			p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;
		}
		else
		{
			unsigned int k_base;
			unsigned int k_limit;

			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_descriptor(&p->ldts[INDEX_LDT_C], 0,
							(k_base + k_limit) >> LIMIT_4K_SHIFT,
							DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);
			init_descriptor(&p->ldts[INDEX_LDT_RW], 0,
							(k_base + k_limit) >> LIMIT_4K_SHIFT,
							DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
		}

		p->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;
		p->regs.ds = p->regs.es = p->regs.fs = p->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip = (unsigned int)t->initial_eip;
		p->regs.esp = (unsigned int)stack;
		p->regs.eflags = eflags;

		p->ticks = p->priority = priority;

		p->flags = 0;
		p->message = 0;
		p->recvfrom = NO_TASK;
		p->sendto = NO_TASK;
		p->has_int_message = 0;
		p->sending = 0;
		p->next = 0;

		for (j = 0; j < NR_FILES; j++)
		{
			p->filp[j] = 0;
		}

		stack -= t->stacksize;
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
	for (i = 0; i < 80 * 25; i++)
	{
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

void untar(const char *filename)
{
	printf("[extract `%s'\n", filename);
	int fd = open(filename, O_RDWR);
	assert(fd != -1);

	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);

	while (1)
	{
		read(fd, buf, SECTOR_SIZE);
		if (buf[0] == 0)
			break;

		struct posix_tar_header *phdr = (struct posix_tar_header *)buf;

		/* calculate the file size */
		char *p = phdr->size;
		int f_len = 0;
		while (*p)
			f_len = (f_len * 8) + (*p++ - '0'); /* octal */

		int bytes_left = f_len;
		int fdout = open(phdr->name, O_CREAT | O_RDWR);
		if (fdout == -1)
		{
			printf("    failed to extract file: %s\n", phdr->name);
			printf(" aborted]\n");
			return;
		}
		printf("    %s (%d bytes)\n", phdr->name, f_len);
		while (bytes_left)
		{
			int iobytes = min(chunk, bytes_left);
			read(fd, buf,
				 ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}

	close(fd);

	printf(" done]\n");
}

void Init3()
{
	int fd_stdin = open("/dev_tty0", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	printf("Init() is running ...\n");

	/* extract `cmd.tar' */
	untar("/cmd.tar");

	int pid = fork();
	if (pid != 0)
	{ /* parent process */
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}
	else
	{ /* child process */
		execl("/echo", "echo", "hello", "world", "this is a test", 0);
	}

	while (1)
	{
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}

	assert(0);
}
void Init()
{
	int fd_stdin = open("/dev_tty0", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	printf("Init() is running...\n");
	untar("/cmd.tar");

	char* tty_list[] = { "/dev_tty0", "/dev_tty1", "/dev_tty2" };
	int i;
	for (i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++) {
		int pid = fork();
		if (pid != 0) {
			printf("parent is running, child pid: %d\n", pid);
		} else {
			printf("child is running, pid: %d\n", getpid());
			close(fd_stdin);
			close(fd_stdout);
			shell(tty_list[i]);
			assert(0);
		}
	}

	while (1)
	{
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}
}

void TestA()
{
	for (;;)
	{
	}

	int fd;
	int i;
	char *filenames[] = {"/foo", "/bar", "/baz"};
	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++)
	{
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		printl("file %s created, fd: %d\n", filenames[i], fd);
		close(fd);
	}

	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++)
	{
		if (unlink(filenames[i]) == 0)
			printl("File removed %s\n", filenames[i]);
		else
			printl("Failed to remove file: %s\n", filenames[i]);
	}

	spin("TestA");
}

void TestB()
{
	for (;;)
	{
	}
	char tty_name[] = "/dev_tty0";

	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];

	while (1)
	{
		printf("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		if (strcmp(rdbuf, "hello") == 0)
			printf("hello world!\n");
		else if (rdbuf[0])
			printf("{%s}\n", rdbuf);
	}

	assert(0); /* never arrive here */
}

void TestC()
{
	for (;;)
	{
	}
}

void shell(const char *tty_name)
{
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);
	char rdbuf[128];

	while (1) {
		write(1, "$ ", 2);
		int r = read(0, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char* argv[PROC_ORIGIN_STACK];
		char* p = rdbuf;
		char* s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) {
				s = p;
				word = 1;
			}
			if ( (*p == ' ' || *p == 0) && word ) {
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while(ch);
		argv[argc] = 0;

		int fd = open(argv[0], O_RDWR);
		if (fd == -1) {
			if (rdbuf[0]) {
				write(1, "{", 1);
				write(1, rdbuf, r);
				write(1, "}\n", 2);
			}
		} else {
			close(fd);
			int pid = fork();
			if (pid != 0) {
				int s;
				wait(&s);
			} else {
				execv(argv[0], argv);
			}
		}
	}
	close(1);
	close(0);
}
