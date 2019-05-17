#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

static msg_send(struct Process* current, int dest, struct Message* m);
static int msg_receive(struct Process* p, int src, struct Message* m);

int sys_sendrec(int func, int src_dest, struct Message* m, struct Process* p)
{
	assert(k_reenter == 0);
	assert( (src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) || src_dest == ANY || src_dest == INTERRUPT );
	
	int ret = 0;
	int caller = proc2pid(p);
	struct Message* mla = (struct Message*)va2la(caller, m);
	mla->source = caller;

	assert(mla->source != src_dest);
	if (func == SEND) {
		ret = msg_send(p, src_dest, m);
		if (ret != 0) return ret;
	} else if (func == RECEIVE) {
		ret = msg_receive(p, src_dest, m);
		if (ret != 0) return ret;
	} else {
		panic("{sys_sendrec} invalid function: %d (SEND:%d, RECEIVE:%d).",
			func, SEND, RECEIVE);
	}

	return 0;
}

static int msg_send(struct Process* current, int dest, struct Message* m)
{
	struct Process * sender = current;
	struct Process* pdest = proc_table + dest;
	assert(proc2pid(sender) != dest);

	return 0;
}

static int msg_receive(struct Process* p, int src, struct Message* m)
{
	return 0;
}

void* va2la(int pid, void* va)
{
	struct Process* p = &proc_table[pid];
	u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	u32 la = seg_base + (u32)va;

	if (pid < NR_TASKS + NR_PROCS) {
		assert(la == (u32)va);
	}

	return (void*)la;
}

int ldt_seg_linear(struct Process* p, int index)
{
	struct Descriptor *d = &p->ldts[index];
	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}