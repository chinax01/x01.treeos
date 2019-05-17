#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

static void init_mm();
static void cleanup(struct Process* p);

void task_mm()
{
	init_mm();
	while (1)
	{
		send_rec(RECEIVE, ANY, &mm_msg);
		int src = mm_msg.source;
		int reply = 1;
		int msgtype = mm_msg.type;
		switch (msgtype)
		{
		case FORK:
			mm_msg.RETVAL = do_fork();
			break;
		case EXIT:
			do_exit(mm_msg.STATUS);
			reply = 0;
			break;
		case EXEC:
			mm_msg.RETVAL = do_exec();
			break;
		case WAIT:
			do_wait();
			reply = 0;
			break;
		default:
			dump_msg("MM::unknown msg", &mm_msg);
			assert(0);
			break;
		}
		if (reply) {
			mm_msg.type = SYSCALL_RET;
			send_rec(SEND, src, &mm_msg);
		}
	}
}

static void init_mm()
{
	struct boot_params bp;
	get_boot_params(&bp);

	memory_size = bp.mem_size;
	printl("{MM}::memsize: %dMB\n", memory_size / (1024 * 1024));
}

int do_fork()
{
	struct Process *p = proc_table;
	int i;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++) {
		if (p->flags == FREE_SLOT) break;
	}
	int child_pid = i;
	assert(p == &proc_table[child_pid]);
	assert(child_pid >= NR_TASKS + NR_NATIVE_PROCS);
	if (i == NR_TASKS + NR_PROCS) return -1;
	assert(i < NR_TASKS + NR_PROCS);

	int pid = mm_msg.source;
	u16 child_ldt_sel = p->ldt_sel;
	*p = proc_table[pid];
	p->ldt_sel = child_ldt_sel;
	p->parent = pid;
	sprintf(p->name, "%s_%d", proc_table[pid].name, child_pid);

	struct Descriptor* desc;
	desc = &proc_table[pid].ldts[INDEX_LDT_C];
	int caller_T_base = reassembly(desc->base_high, 24, desc->base_mid, 16, desc->base_low);
	int caller_T_limit = reassembly(0, 0, (desc->limit_high_attr2 & 0xF), 16, desc->limit_low);
	int caller_T_size = ( (caller_T_limit+1) * ((desc->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1) );
	desc = &proc_table[pid].ldts[INDEX_LDT_RW];
	int caller_D_S_base = reassembly(desc->base_high, 24, desc->base_mid, 16, desc->base_low);
	int caller_D_S_limit = reassembly((desc->limit_high_attr2 & 0xF), 16, 0, 0, desc->limit_low);
	int caller_D_S_size = ((caller_T_limit + 1) * ((desc->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1) );
	assert( (caller_T_base == caller_D_S_base) 
		&& (caller_T_limit == caller_D_S_limit) && (caller_T_size == caller_D_S_size) );
	
	int child_base = alloc_mem(child_pid, caller_T_size);
	printl("MM 0x%x <- 0x%x(0x%x bytes)\n", child_base, caller_T_base, caller_T_size);
	phys_copy((void*)child_base, (void*)caller_T_base, caller_T_size);

	init_descriptor(&p->ldts[INDEX_LDT_C], child_base, 
		(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);
	init_descriptor(&p->ldts[INDEX_LDT_RW], child_base,
		(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);

	struct Message msg2fs;
	msg2fs.type = FORK;
	msg2fs.PID = child_pid;
	send_rec(BOTH, TASK_FS, &msg2fs);

	mm_msg.PID = child_pid;

	struct Message m;
	m.type = SYSCALL_RET;
	m.RETVAL = 0;
	m.PID = 0;
	send_rec(SEND, child_pid, &m);

	return 0;
}

int alloc_mem(int pid, int memsize)
{
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
	if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
		panic("unsupporte memory request: %d (should be less than %d)", memsize, PROC_IMAGE_SIZE_DEFAULT);
	}
	int base = PROCS_BASE + (pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;
	if (base + memsize >= memory_size)
		panic("memory allocation failed. pid: %d", pid);

	return base;
}

void do_wait()
{
	int pid = mm_msg.source;
	int i;
	int children = 0;
	struct Process* p = proc_table;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++) {
		if (p->parent == pid) {
			children++;
			if (p->flags & HANGING) {
				cleanup(p);
				return;
			}
		}
	}

	if (children) {
		proc_table[pid].flags |= WAITING;
	} else {
		struct Message m;
		m.type = SYSCALL_RET;
		m.PID = NO_TASK;
		send_rec(SEND, pid, &m);
	}
}

static void cleanup(struct Process* p)
{
	struct Message m2parent;
	m2parent.type = SYSCALL_RET;
	m2parent.PID = proc2pid(p);
	m2parent.STATUS = p->exit_status;
	send_rec(SEND, p->parent, &m2parent);

	p->flags = FREE_SLOT;
}

void do_exit(int status)
{
	int i;
	int pid = mm_msg.source;
	int parent = proc_table[pid].parent;
	struct Process* p = &proc_table[pid];

	struct Message m2fs;
	m2fs.type = EXIT;
	m2fs.PID = pid;
	send_rec(BOTH, TASK_FS, &m2fs);

	free_mem(pid);

	p->exit_status = status;

	if (proc_table[parent].flags & WAITING) {
		proc_table[parent].flags &= ~WAITING;
		cleanup(&proc_table[pid]);
	} else {
		proc_table[pid].flags |= HANGING;
	}

	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		if (proc_table[i].parent == pid) {
			proc_table[i].parent = INIT;
			if ((proc_table[INIT].flags & WAITING) && (proc_table[i].flags & HANGING)) {
				proc_table[INIT].flags &= ~WAITING;
				cleanup(&proc_table[i]);
			}
		}
	}
}

int free_mem(int pid)
{
	return 0;
}