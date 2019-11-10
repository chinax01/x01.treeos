#define PAGE_SIZE	1024
long user_stack[PAGE_SIZE>>2];
struct {
	long *a; 
	long b;
} stack_start = { &user_stack[PAGE_SIZE>>2], 0x10 };
