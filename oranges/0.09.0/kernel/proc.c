#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

static msg_send(struct Process *current, int dest, struct Message *m);
static int msg_receive(struct Process *p, int src, struct Message *m);
static void block(struct Process *p);
static void unblock(struct Process *p);
static int deadlock(int src, int dest);

void schedule()
{
	struct Process *p;
	int greatest_ticks = 0;

	while (!greatest_ticks)
	{
		for (p = &FIRST_PROC; p <= &LAST_PROC; p++)
		{
			if (p->flags == 0)
			{
				if (p->ticks > greatest_ticks)
				{
					greatest_ticks = p->ticks;
					proc_ready = p;
				}
			}
		}
		if (!greatest_ticks)
		{
			for (p = &FIRST_PROC; p <= &LAST_PROC; p++)
			{
				if (p->flags == 0)
					p->ticks = p->priority;
			}
		}
	}
}


int send_rec(int func, int src_dest, struct Message *m)
{
	int ret = 0;

	if (func == RECEIVE)
		memset(m, 0, sizeof(struct Message));

	switch (func)
	{
	case BOTH:
		ret = sendrec(SEND, src_dest, m);
		if (ret == 0)
			ret = sendrec(RECEIVE, src_dest, m);
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(func, src_dest, m);
	default:
		assert(func == BOTH || func == SEND || func == RECEIVE);
		break;
	}

	return ret;
}

int sys_sendrec(int function, int src_dest, struct Message* m, struct Process* p)
{
	assert(k_reenter == 0);	/* make sure we are not in ring0 */
	assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) ||
	       src_dest == ANY ||
	       src_dest == INTERRUPT);

	int ret = 0;
	int caller = proc2pid(p);
	struct Message* mla = (struct Message*)va2la(caller, m);
	mla->source = caller;

	assert(mla->source != src_dest);

	/**
	 * Actually we have the third message type: BOTH. However, it is not
	 * allowed to be passed to the kernel directly. Kernel doesn't know
	 * it at all. It is transformed into a SEND followed by a RECEIVE
	 * by `send_rec()'.
	 */
	if (function == SEND) {
		ret = msg_send(p, src_dest, m);
		if (ret != 0)
			return ret;
	}
	else if (function == RECEIVE) {
		ret = msg_receive(p, src_dest, m);
		if (ret != 0)
			return ret;
	}
	else {
		panic("{sys_sendrec} invalid function: "
		      "%d (SEND:%d, RECEIVE:%d).", function, SEND, RECEIVE);
	}

	return 0;
}

int sys_sendrec2(int func, int src_dest, struct Message *m, struct Process *p)
{
	assert(k_reenter == 0);
	assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) 
		|| src_dest == ANY || src_dest == INTERRUPT);

	int ret = 0;
	int caller = proc2pid(p);
	struct Message *mla = (struct Message *)va2la(caller, m);
	mla->source = caller;

	assert(mla->source != src_dest);
	if (func == SEND)
	{
		ret = msg_send(p, src_dest, m);
		if (ret != 0)
			return ret;
	}
	else if (func == RECEIVE)
	{
		ret = msg_receive(p, src_dest, m);
		if (ret != 0)
			return ret;
	}
	else
	{
		panic("{sys_sendrec} invalid function: %d (SEND:%d, RECEIVE:%d).",
			  func, SEND, RECEIVE);
	}

	return 0;
}

void reset_msg(struct Message *m)
{
	memset(m, 0, sizeof(struct Message));
}

static int msg_send(struct Process *current, int dest, struct Message *m)
{
	struct Process *sender = current;
	struct Process *p_dest = proc_table + dest;
	assert(proc2pid(sender) != dest);

	if (deadlock(proc2pid(sender), dest))
	{
		panic(">>Deadlock<< %s->%s", sender->name, p_dest->name);
	}

	if ((p_dest->flags & RECEIVING) && (p_dest->recvfrom == proc2pid(sender) || p_dest->recvfrom == ANY))
	{
		assert(p_dest->message);
		assert(m);
		phys_copy(va2la(dest, p_dest->message), va2la(proc2pid(sender), m), sizeof(struct Message));
		p_dest->message = 0;
		p_dest->flags &= ~RECEIVING;
		p_dest->recvfrom = NO_TASK;
		unblock(p_dest);

		assert(p_dest->flags == 0);
		assert(p_dest->message == 0);
		assert(p_dest->recvfrom == NO_TASK);
		assert(p_dest->sendto == NO_TASK);
		assert(sender->flags == 0);
		assert(sender->message == 0);
		assert(sender->recvfrom == NO_TASK);
		assert(sender->sendto == NO_TASK);
	}
	else
	{
		sender->flags |= SENDING;
		assert(sender->flags == SENDING);
		sender->sendto = dest;
		sender->message = m;

		struct Process *p;
		if (p_dest->sending)
		{
			p = p_dest->sending;
			while (p->next)
				p = p->next;
			p->next = sender;
		}
		else
		{
			p_dest->sending = sender;
		}
		sender->next = 0;

		block(sender);

		assert(sender->flags == SENDING);
		assert(sender->message != 0);
		assert(sender->recvfrom == NO_TASK);
		assert(sender->sendto == dest);
	}

	return 0;
}

static int msg_receive(struct Process *current, int src, struct Message *m)
{
	struct Process *receiver = current;
	struct Process *p_from = 0;
	struct Process *prev = 0;
	int copyok = 0;

	assert(proc2pid(receiver) != src);

	if (receiver->has_int_message && (src == ANY || src == INTERRUPT))
	{
		struct Message msg;
		reset_msg(&msg);
		msg.source = INTERRUPT;
		msg.type = HARD_INT;
		assert(m);
		phys_copy(va2la(proc2pid(receiver), m), &msg, sizeof(struct Message));
		receiver->has_int_message = 0;

		assert(receiver->flags == 0);
		assert(receiver->message == 0);
		assert(receiver->sendto == NO_TASK);
		assert(receiver->has_int_message == 0);

		return 0;
	}

	if (src == ANY)
	{
		if (receiver->sending)
		{
			p_from = receiver->sending;
			copyok = 1;

			assert(receiver->flags == 0);
			assert(receiver->message == 0);
			assert(receiver->recvfrom == NO_TASK);
			assert(receiver->sendto == NO_TASK);
			assert(receiver->sending != 0);
			assert(p_from->flags == SENDING);
			assert(p_from->message != 0);
			assert(p_from->recvfrom == NO_TASK);
			assert(p_from->sendto == proc2pid(receiver));
		}
	}
	else
	{
		p_from = &proc_table[src];
		if ((p_from->flags & SENDING) && (p_from->sendto == proc2pid(receiver)))
		{
			copyok = 1;
			struct Process *p = receiver->sending;
			assert(p);
			while (p)
			{
				assert(p_from->flags & SENDING);
				if (proc2pid(p) == src)
				{
					p_from = p;
					break;
				}
				prev = p;
				p = p->next;
			}

			assert(receiver->flags == 0);
			assert(receiver->message == 0);
			assert(receiver->recvfrom == NO_TASK);
			assert(receiver->sendto == NO_TASK);
			assert(receiver->sending != 0);
			assert(p_from->flags == SENDING);
			assert(p_from->message != 0);
			assert(p_from->recvfrom == NO_TASK);
			assert(p_from->sendto == proc2pid(receiver));
		}
	}

	if (copyok)
	{
		if (p_from == receiver->sending)
		{
			assert(prev == 0);
			receiver->sending = p_from->next;
			p_from->next = 0;
		}
		else
		{
			assert(prev);
			prev->next = p_from->next;
			p_from->next = 0;
		}
		assert(m);
		assert(p_from->message);
		phys_copy(va2la(proc2pid(receiver), m), va2la(proc2pid(p_from), p_from->message), sizeof(struct Message));
		p_from->message = 0;
		p_from->sendto = NO_TASK;
		p_from->flags &= ~SENDING;
		unblock(p_from);
	}
	else
	{
		receiver->flags |= RECEIVING;
		receiver->message = m;
		if (src == ANY)
			receiver->recvfrom = ANY;
		else
			receiver->recvfrom = proc2pid(p_from);

		block(receiver);

		assert(receiver->flags == RECEIVING);
		assert(receiver->message != 0);
		assert(receiver->recvfrom != NO_TASK);
		assert(receiver->sendto == NO_TASK);
		assert(receiver->has_int_message == 0);
	}

	return 0;
}

void *va2la(int pid, void *va)
{
	struct Process *p = &proc_table[pid];
	u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	u32 la = seg_base + (u32)va;

	if (pid < NR_TASKS + NR_NATIVE_PROCS)
	{
		assert(la == (u32)va);
	}

	return (void *)la;
}

int ldt_seg_linear(struct Process *p, int index)
{
	struct Descriptor *d = &p->ldts[index];
	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

static void block(struct Process *p)
{
	assert(p->flags);
	schedule();
}

static void unblock(struct Process *p)
{
	assert(p->flags == 0);
}

static int deadlock(int src, int dest)
{
	struct Process *p = proc_table + dest;
	while (1)
	{
		if (p->flags & SENDING)
		{
			if (p->sendto == src)
			{
				p = proc_table + dest;
				printl("=_= %s", p->name);
				do
				{
					assert(p->message);
					p = proc_table + p->sendto;

					printl("--> %s", p->name);
				} while (p != proc_table + p->sendto);
				printl("+_+");
				return 1;
			}
			p = proc_table + p->sendto;
		}
		else
		{
			break;
		}
	}
}

void dump_msg(const char * title, struct Message* m)
{
	int packed = 0;
	printl("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",  //, (0x%x, 0x%x, 0x%x)}",
	       title,
	       (int)m,
	       packed ? "" : "\n        ",
	       proc_table[m->source].name,
	       m->source,
	       packed ? " " : "\n        ",
	       m->type,
	       packed ? " " : "\n        ",
	       m->u.m3.m3i1,
	       m->u.m3.m3i2,
	       m->u.m3.m3i3,
	       m->u.m3.m3i4,
	       (int)m->u.m3.m3p1,
	       (int)m->u.m3.m3p2,
	       packed ? "" : "\n",
	       packed ? "" : "\n"/* , */
		);
}

void inform_int(int task_nr)
{
	struct Process* p = proc_table + task_nr;
	if ( (p->flags & RECEIVING) && 
		((p->recvfrom == INTERRUPT) || (p->recvfrom == ANY)) ) {
		p->message->source = INTERRUPT;
		p->message->type = HARD_INT;
		p->message = 0;
		p->has_int_message = 0;
		p->flags &= ~RECEIVING;
		p->recvfrom = NO_TASK;
		assert(p->flags == 0);
		unblock(p);

		assert(p->flags == 0);
		assert(p->message == 0);
		assert(p->recvfrom == NO_TASK);
		assert(p->sendto == NO_TASK);
	} else {
		p->has_int_message = 1;
	}
}