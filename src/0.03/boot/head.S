# boot/head.S

.text
.globl gdt, idt, pg_dir, start_32
pg_dir:
start_32:
	movl $0x10, %eax 
	mov %ax, %ds 
	mov %ax, %es 
	mov %ax, %fs 
	mov %ax, %gs 
	lss stack_start, %esp 	# stack_start at sched.c 
	call setup_idt 
	call setup_gdt
	movl $0x10, %eax 
	mov %ax, %ds 
	mov %ax, %es 
	mov %ax, %fs 
	mov %ax, %gs
	lss stack_start, %esp 
	
	# check A20
	xorl %eax, %eax 
1: 	incl %eax 
	movl %eax, 0x000000
	cmpl %eax, 0x100000		# address: 1M
	je 1b

	# check math chip
	movl %cr0, %eax 
	andl $0x80000011, %eax 	# PG,ET,PE
	testl $0x10, %eax 
	jne 1f			# ET is set - 387 is present
	orl $4, %eax 	# else set emulate bit
1:	movl %eax, %cr0

	jmp after_page_tables 

setup_gdt:
	lgdt gdt_descr
	ret

setup_idt:
	lea ignore_int, %edx 
	movl $0x00080000, %eax 
	movw %dx, %ax 
	movw $0x8e00, %dx 
	lea idt, %edi 
	mov $256, %ecx 
rp_sidt:
	movl %eax, (%edi)
	movl %edx, 4(%edi)
	addl $8, %edi 
	dec %ecx 
	jne rp_sidt
	lidt idt_descr
	ret

	.org 0x1000
pg0:
	.org 0x2000
pg1:
	.org 0x3000
pg3:
	.org 0x4000
after_page_tables:
	pushl $0
	pushl $0
	pushl $0
	pushl $L6
	pushl $main
	jmp setup_paging
L6:
	jmp L6

	.align 4
setup_paging:
	ret
	
	.align 4
ignore_int:
	incb 0xb8000 + 160
	movb $2, 0xb8000+161
	iret 

	.align 4
	.word 0
idt_descr:
	.word 256*8-1
	.long idt 

	.align 4
	.word 0
gdt_descr:
	.word 256*8-1
	.long gdt 

	.align 8
idt:
	.fill 256,8,0

gdt:
	.quad 0x0000000000000000
	.quad 0x00c09a00000007ff
	.quad 0x00c09200000007ff
	.quad 0x0000000000000000
	.fill 252,8,0