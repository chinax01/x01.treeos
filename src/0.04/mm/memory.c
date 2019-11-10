#include <signal.h>

#include <linux/config.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <asm/system.h>

int do_exit(long code);

// 重新加载页目录基址寄存器 cr3 来刷新高速缓冲
#define invalidate()	__asm__("movl %%eax, %%cr3"::"a"(0))

#if (BUFFER_END < 0x100000)
#define LOW_MEM		0x100000
#else 
#define LOW_MEM		BUFFER_END
#endif 

#define PAGING_MEMORY		(HIGH_MEMORY - LOW_MEM)
#define PAGING_PAGES		(PAGING_MEMORY / 4096)
#define MAP_NR(addr)		(((addr) - LOW_MEM) >> 12)

#if (PAGING_PAGES < 10)
#error "Won't work" 
#endif 

#define copy_page(from,to)	__asm__("cld; rep; movsl"::"S"(from),"D"(to),"c"(1024))

static unsigned short mem_map[PAGING_PAGES] = {0, };

// 在主内存获取空闲页面
unsigned long get_free_page(void)
{
	register unsigned long __res asm("ax");

	__asm__("std; repne; scasw\n\t"
		"jne 1f\n\t"
		"movw $1, 2(%%edi)\n\t"
		"sall $12, %%ecx\n\t"
		"movl %%ecx, %%edx\n\t"
		"addl %2, %%edx\n\t"
		"movl $1024, %%ecx\n\t"
		"leal 4092(%%edx), %%edi\n\t"
		"rep; stosl\n\t"
		"movl %%edx, %%eax\n"
		"1:"
		:"=a"(__res)
		:"0"(0),"i"(LOW_MEM), "c"(PAGING_PAGES), "D"(mem_map + PAGING_PAGES - 1) 
	);
	return __res;
}

void free_page(unsigned long addr) 
{
	if (addr < LOW_MEM) return;
	if (addr > HIGH_MEMORY) 
		panic("trying to free nonexistent page");
	addr -= LOW_MEM;
	addr >>= 12;
	if (mem_map[addr]--) return;
	mem_map[addr] = 0;
	panic("trying to free free-page");
}

int free_page_tables(unsigned long from,unsigned long size)
{
	unsigned long *pg_table;
	unsigned long *dir, nr;

	if (from & 0x3fffff)
		panic("free_page_tables called with wrong alignemnt");
	if (!from)
		panic("Trying to free up swapper memory space");
	size = (size + 0x3fffff) >> 22;
	dir = (unsigned long*) ((from >> 20) & 0xffc);
	for (; size-- > 0; dir++) {
		if (!(1 & *dir)) continue;
		pg_table = (unsigned long*) (0xfffff000 & *dir);
		for (nr=0; nr<1024; nr++) {
			if (1 & *pg_table)
				free_page(0xfffff000 & *pg_table);
			*pg_table = 0;
			pg_table++;
		}
		free_page(0xfffff000 & *dir);
		*dir = 0;
	}
	invalidate();
	return 0;
}

// 复制页目录表项和页表项
int copy_page_tables(unsigned long from,unsigned long to,long size) 
{
	unsigned long * from_page_table;
	unsigned long * to_page_table;
	unsigned long this_page;
	unsigned long *from_dir, *to_dir;
	unsigned long nr;

	if ((from & 0x3fffff) || (to & 0x3fffff))
		panic("copy_page_table called with wrong alignment");
	from_dir = (unsigned long*) ((from>>20) & 0xffc);
	to_dir = (unsigned long*) ((to>>20) & 0xffc);
	size = ((unsigned)(size + 0x3fffff)) >> 22;
	for (; size-- > 0; from_dir++, to_dir++) {
		if (1 & *to_dir)
			panic("copy_page_table: already exist");
		if (!(1 & *from_dir)) continue;
		from_page_table = (unsigned long*) (0xfffff000 & *from_dir);
		if (!(to_page_table = (unsigned long*)get_free_page()))
			return -1;
		*to_dir = ((unsigned long)to_page_table) | 7;
		nr = (from == 0) ? 0xa0 : 1024;
		for (; nr-- > 0; from_page_table++, to_page_table++) {
			this_page = * from_page_table;
			if (!(1 & this_page)) continue;
			this_page &= ~2;
			*to_page_table = this_page;
			if (this_page > LOW_MEM) {
				*from_page_table = this_page;
				this_page -= LOW_MEM;
				this_page >>= 12;
				mem_map[this_page]++;
			}
		}
	}
	invalidate();
	return 0;
}

unsigned long put_page(unsigned long page, unsigned long address)
{
	unsigned long tmp, *page_table;

	if (page < LOW_MEM || page > HIGH_MEMORY)
		printk("Trying to put page %p at %p\n", page, address);
	if (mem_map[(page - LOW_MEM) >> 12] != 1)
		printk("mem_map disagress with %p at %p\n", page, address);
	page_table = (unsigned long*) ((address>>20) & 0xffc);
	if ((*page_table) & 1)
		page_table = (unsigned long*) (0xfffff000 & *page_table);
	else {
		if (!(tmp = get_free_page())) return 0;
		*page_table = tmp | 7;
		page_table = (unsigned long*)tmp;
	}
	page_table[(address>>12) & 0x3ff] = page | 7;
	return page;
}

void un_wp_page(unsigned long* table_entry)
{
	unsigned long old_page, new_page;

	old_page = 0xfffff000 & *table_entry;
	if (old_page >= LOW_MEM && mem_map[MAP_NR(old_page)] == 1) {
		*table_entry |= 2;
		return;
	}
	if (!(new_page = get_free_page()))
		do_exit(SIGSEGV);
	if (old_page >= LOW_MEM)
		mem_map[MAP_NR(old_page)]--;
	copy_page(old_page, new_page);
	*table_entry = new_page | 7;
	copy_page(old_page, new_page);
}

void do_wp_page(unsigned long error_code, unsigned long address)
{
	un_wp_page( (unsigned long*) (((address>>10) & 0xffc) 
		+ (0xfffff000 & *((unsigned long*)((address>>20) & 0xffc)))) );
}

void write_verify(unsigned long address)
{
	unsigned long page;

	// page: 页目录项对应的页表
	if ( !( (page = *((unsigned long*)((address>>20) & 0xffc))) & 1 ) )
		return;
	page &= 0xfffff000;	
	page += ((address>>10) & 0xffc);
	if ((3 & *(unsigned long*)page) == 1)
		un_wp_page((unsigned long*) page);
	return;
}

void do_no_page(unsigned long error_code, unsigned long address)
{
	unsigned long tmp;

	if (tmp = get_free_page()) {
		if (put_page(tmp, address)) return;
	}

	do_exit(SIGSEGV);
}

void calc_mem(void)
{
	int i, j, k, free=0;
	long *pg_tbl;

	for (i=0; i<PAGING_PAGES; i++) {
		if (!mem_map[i]) free++;
	}
	printk("%d pages free (of %d)\n\r", free, PAGING_PAGES);
	for (i=2; i<1024; i++) {
		if (1 & pg_dir[i]) {
			pg_tbl = (long*) (0xfffff000 & pg_dir[i]);
			for (j=k=0; j<1024; j++) {
				if (pg_tbl[j] & 1) k++;
			}
			printk("pg-dir[%d] uses %d pages\n", i, k);
		}
	}
}
