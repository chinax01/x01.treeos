#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>

void cstart()
{
	disp_str("\n\n\n\n\n\n\n-------------cstart beging---------------\n");

	memcpy(&gdt, (void*)( *((unsigned int*)(&gdt_ptr[2])) ), 
		*((unsigned short*)(&gdt_ptr[0])) + 1);
	unsigned short* limit = (unsigned short*)(&gdt_ptr[0]);
	unsigned int* base = (unsigned int*)(&gdt_ptr[2]);
	*limit = GDT_SIZE * sizeof(struct Descriptor) - 1;
	*base = &gdt;

	unsigned short* idt_limit = (unsigned short*)(&idt_ptr[0]);
	unsigned int* idt_base = (unsigned int*)(&idt_ptr[2]);
	*idt_limit = IDT_SIZE * sizeof(struct Gate) - 1;
	*idt_base = (unsigned int)&idt;

	init_protect();

	disp_str("------------cstart end-------------\n");
}