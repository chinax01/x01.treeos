#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <elf.h>

int get_kernel_map(unsigned int *base, unsigned int* limit)
{
	struct boot_params bp;
	get_boot_params(&bp);
	Elf32_Ehdr* elf_header = (Elf32_Ehdr*)(bp.kernel_file);
	if (memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0)
		return -1;

	*base = ~0;
	unsigned int t = 0;
	int i;
	for (i = 0; i < elf_header->e_shnum; i++) {
		Elf32_Shdr* section_header = (Elf32_Shdr*)(bp.kernel_file 
			+ elf_header->e_shoff + i * elf_header->e_shentsize);
		if (section_header->sh_flags & SHF_ALLOC) {
			int bottom = section_header->sh_addr;
			int top = section_header->sh_addr + section_header->sh_size;
			if (*base > bottom) *base = bottom;
			if (t < top) t = top;
		}
	}
	assert(*base < t);
	*limit = t - *base - 1;
	return 0;
}

void get_boot_params(struct boot_params* bp)
{
	int* p = (int*)BOOT_PARAM_ADDR;
	assert(p[BI_MAG] == BOOT_PARAM_MAGIC);
	bp->mem_size = p[BI_MEM_SIZE];
	bp->kernel_file = (unsigned char*)(p[BI_KERNEL_FILE]);
	assert(memcmp(bp->kernel_file, ELFMAG, SELFMAG) == 0);
}

char* itoa(char* s, int n)
{
	char* p = s;
	char c;
	int i;
	int flag = 0;

	*p++ = '0';
	*p++ = 'x';
	if (n == 0) {
		*p++ = '0';
	} else {
		for (i = 28; i >= 0; i -= 4) {
			c = (n >> i) & 0xF;
			if (flag || (c > 0)) {
				flag = 1;
				c += '0';
				if (c > '9') {
					c += 7;
				}
				*p++ = c;
			}
		}
	}
	*p = 0;
	
	return s;
}

void disp_int(int n)
{
	char buf[16];
	itoa(buf, n);
	disp_str(buf);
}

void assertion_failure(char* exp, char* file, char* base_file, int line)
{
	printl("%c  assert(%s) failed, file: %s, base_file: %s, line: %d",
		MAG_CH_ASSERT, exp, file, base_file, line);
	spin("assertion_failure()");
	__asm__ __volatile__("ud2");
}

void panic(const char* fmt, ...)
{
	//int i;
	char buf[256];

	va_list arg = (va_list)((char*)&fmt + 4);
	vsprintf(buf, fmt, arg);

	printl("%c panic! %s", MAG_CH_PANIC, buf);

	__asm__ __volatile__("ud2");
}

void spin(const char* func_name)
{
	printl("\nspinning in %s ...\n", func_name);
	while(1) { }
}

int memcmp(const void * s1, const void *s2, int n)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = (const char *)s1;
	const char * p2 = (const char *)s2;
	int i;
	for (i = 0; i < n; i++,p1++,p2++) {
		if (*p1 != *p2) {
			return (*p1 - *p2);
		}
	}
	return 0;
}

int strcmp(const char* s1, const char* s2)
{
	if (s1 == 0 || s2 == 0) return s1 - s2;
	const char* p1 = s1;
	const char* p2 = s2;
	for (; *p1 && *p2; p1++, p2++) {
		if (*p1 != *p2) break;
	}
	return *p1 - *p2;
}