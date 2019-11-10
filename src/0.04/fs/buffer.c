#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/system.h>

#if (BUFFER_END & 0xffff)
#error "Bad BUFFER_END value" 
#endif 

#if (BUFFER_END > 0xa000 && BUFFER_END <= 0x100000)
#error "Bad BUFFER_END value"
#endif 

extern int end;
struct buffer_head* start_buffer = (struct buffer_head*)&end;
struct buffer_head *hash_table[NR_HASH];
static struct buffer_head *free_list;
static struct task_struct *buffer_wait = NULL;
int NR_BUFFERS = 0;

static inline void wait_on_buffer(struct buffer_head *bh)
{
	cli();
	while (bh->b_lock)
		sleep_on(bh->b_wait);
	sti();
}

int sys_sync(void)
{
	int i;
	struct buffer_head *bh;

	sync_inodes();
	bh = start_buffer;
	for (i=0; i<NR_BUFFERS; i++, bh++) {
		wait_on_buffer(bh);
		if (bh->b_dirt)
			ll_rw_block(WRITE, bh);
	}
	return 0;
}

static int sync_dev(int dev)
{
	int i;
	struct buffer_head* bh;

	bh = start_buffer;
	for (i=0; i<NR_BUFFERS; i++, bh++) {
		if (bh->b_dev != dev) continue;
		wait_on_buffer(bh);
		if (bh->b_dirt)
			ll_rw_block(WRITE, bh);
	}
	return 0;
}

#define _hashfn(dev, block)		( ((unsigned)(dev ^ block)) % NR_HASH )
#define hash(dev, block)		hash_table[_hashfn(dev, block)]

static inline void remove_from_queues(struct buffer_head* bh)
{
	if (bh->b_next)
		bh->b_next->b_prev = bh->b_prev;
	if (bh->b_prev)
		bh->b_prev->b_next = bh->b_next;
	if (hash(bh->b_dev, bh->b_blocknr) == bh)
		hash(bh->b_dev, bh->b_blocknr) = bh->b_next;

	if (!(bh->b_prev_free) || !(bh->b_next_free))
		panic("Free_block list corrupted");
	bh->b_prev_free->b_next_free = bh->b_next_free;
	bh->b_next_free->b_prev_free = bh->b_prev_free;
	if (free_list == bh)
		free_list = bh->b_next_free;
}

static inline void insert_into_queues(struct buffer_head *bh)
{
	bh->b_next_free = free_list;
	bh->b_prev_free = free_list->b_prev_free;
	free_list->b_prev_free->b_next_free = bh;
	free_list->b_prev_free = bh;

	bh->b_prev = NULL;
	bh->b_next = NULL;
	if (!bh->b_dev) return;
	bh->b_next = hash(bh->b_dev, bh->b_blocknr);
	hash(bh->b_dev, bh->b_blocknr) = bh;
	bh->b_next->b_prev = bh;
}

static struct buffer_head* find_buffer(int dev, int block)
{
	struct buffer_head* tmp;

	for (tmp = hash(dev, block); tmp != NULL; tmp = tmp->b_next)
		if (tmp->b_dev == dev && tmp->b_blocknr == block)
			return tmp;
	return NULL;
}

struct buffer_head* get_hash_table(int dev, int block)
{
	struct buffer_head* bh;

repeat:
	if (!(bh = find_buffer(dev, block)))
		return NULL;
	bh->b_count++;
	wait_on_buffer(bh);
	if (bh->b_dev != dev || bh->b_blocknr != block) {
		brelse(bh);
		goto repeat;
	}
	return bh;
}

struct buffer_head* getblk(int dev, int block)
{
	struct buffer_head* tmp;

repeat:
	if (tmp = get_hash_table(dev, block))
		return tmp;
	tmp = free_list;
	do {
		if (!tmp->b_count) {
			wait_on_buffer(tmp);
			if (!tmp->b_count) break;
		}
		tmp = tmp->b_next_free;
	} while (tmp != free_list || (tmp = NULL));

	if (!tmp) {
		printk("Sleeping on free buffer..");
		sleep_on(&buffer_wait);
		printk("ok\n");
		goto repeat;
	}
	tmp->b_count++;
	remove_from_queues(tmp);

	if (tmp->b_dirt)
		sync_dev(tmp->b_dev);
	tmp->b_dev = dev;
	tmp->b_blocknr = block;
	tmp->b_dirt = 0;
	tmp->b_uptodate = 0;
	if (find_buffer(dev, block)) {
		tmp->b_dev = 0;
		tmp->b_blocknr = 0;
		tmp->b_count = 0;
		insert_into_queues(tmp);
		goto repeat;
	}

	insert_into_queues(tmp);
	return tmp;
}

void brelse(struct buffer_head *buf)
{
	if (!buf) return ;
	wait_on_buffer(buf);
	if (!(buf->b_count--))
		panic("Trying to free free-buffer");
	wake_up(&buffer_wait);
}

struct buffer_head* bread(int dev, int block)
{
	struct buffer_head *bh;

	if (!(bh = getblk(dev, block))) 
		panic("bread: getblk return NULL\n");
	if (bh->b_uptodate) return bh;
	ll_rw_block(READ, bh);
	if (bh->b_uptodate) return bh;
	brelse(bh);
	return NULL;
}

void buffer_init(void) 
{
	struct buffer_head *h = start_buffer;
	void* b = (void*) BUFFER_END;
	int i;

	while ( (b -= BLOCK_SIZE) >= ((void*)(h+10))) {
		h->b_dev = 0;
		h->b_dirt = 0;
		h->b_count = 0;
		h->b_lock = 0;
		h->b_uptodate = 0;
		h->b_wait = NULL;
		h->b_next = NULL;
		h->b_prev = NULL;
		h->b_data = (char*)b;
		h->b_prev_free = h-1;
		h->b_next_free = h+1;
		h++;
		NR_BUFFERS++;
		if (b == (void*)0x100000) {
			b = (void*)0xa0000;
		}
	}
	h--;
	free_list = start_buffer;
	free_list->b_prev_free = h;
	h->b_next_free = free_list;
	for (i=0; i<NR_HASH; i++) {
		hash_table[i] = NULL;
	}
}
