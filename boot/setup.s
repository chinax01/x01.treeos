.code16

.equ INITSEG, 0x9000
.equ SYSSEG, 0x1000
.equ SETUPSEG, 0x9020

.global _start
.text

	ljmp $SETUPSEG, $_start
_start:
	mov %cs, %ax 
	mov %ax, %ds 
	mov %ax, %es 
	lss stk, %esp 
	# Print message
	mov $0x03, %ah 
	xor %bh, %bh
	int $0x10

	mov $17, %cx 
	mov $0x000c, %bx 
	mov $msg, %bp 
	mov $0x1301, %ax 
	int $0x10

	# ds = 0x9000
	mov $INITSEG, %ax 
	mov %ax, %ds 

	# Get cursor pos
	mov $0x03, %ah 
	xor %bh, %bh 
	int $0x10
	mov %dx, %ds:0

	# Get memory size
	mov $0x88, %ah 
	int $0x15
	mov %ax, %ds:2

	# Get video card data
	mov $0x0f, %ah 
	int $0x10
	mov %bx, %ds:4
	mov %ax, %ds:6

	# Check fro EGA/VGA
	mov $0x12, %ah 
	mov $0x10, %bl 
	int $0x10
	mov %ax, %ds:8
	mov %bx, %ds:10
	mov %cx, %ds:12

	mov $0x5019, %ax 
	cmp $0x10, %bl 
	je novga
	#call chsvga
novga:
	mov %ax, %ds:14

	# Get hd0 data
	mov $0x0000, %ax 
	mov %ax, %ds 
	mov $4*0x41, %si
	mov $INITSEG, %ax 
	mov %ax, %es 
	mov $0x0080, %di 
	mov $0x10, %cx 
	rep
	movsb 

	#Get hd1 data
	mov $0x0000, %ax 
	mov %ax, %ds 
	mov $4*0x46, %si 
	mov $INITSEG, %ax 
	mov %ax, %es 
	mov $0x0090, %di 
	mov $0x10, %cx 
	rep 
	movsb 

	# Check there is a hd1
	mov $0x1500, %ax 
	mov $0x81, %dl 
	int $0x13
	jc no_disk1 
	cmp $3, %ah 
	je is_disk1 
no_disk1:
	mov $INITSEG, %ax 
	mov %ax, %es 
	mov $0x0090, %di 
	mov $0x10, %cx 
	mov $0x00, %ax 
	rep 
	stosb 
is_disk1:

# Protect mode
	cli 

	mov $0x0000, %ax 
	cld 
do_move:
	mov %ax, %es 
	add $0x1000, %ax 
	cmp $0x9000, %ax 
	jz end_move 
	mov %ax, %ds 
	sub %di, %di 
	sub %si, %si 
	mov $0x8000, %cx 
	rep 
	movsw 
	jmp do_move
end_move:
	mov $SETUPSEG, %ax 
	mov %ax, %ds 
	lidt idt_48
	lgdt gdt_48

	# A20
	inb $0x92, %al 
	orb $0b00000010, %al 
	outb %al, $0x92

	# 8259A
	mov $0x11, %al 			# ICW1: 边缘触发，多片，需要ICW4
	out %al, $0x20 
	.word 0x00eb, 0x00eb 
	out %al, $0xa0
	.word 0x00eb, 0x00eb 
	mov $0x20, %al 			# ICW2： 0x20-0x27
	out %al, $0x21
	.word 0x00eb, 0x00eb 
	mov $0x28, %al 			# slave 0x28-0x2f
	out %al, $0xa1
	.word 0x00eb, 0x00eb 
	mov $0x04, %al 			# ICW3: IR2 级联从片
	out %al, $0x21 
	.word 0x00eb, 0x00eb 
	mov $0x02, %al 			# 级联的主片号
	out %al, $0xa1
	.word 0x00eb, 0x00eb 
	mov $0x01, %al 			# ICW4： 80x86 模式
	out %al, $0x21
	.word 0x00eb, 0x00eb 
	out %al, $0xa1 
	.word 0x00eb, 0x00eb 
	mov $0xff, %al 			# OCW1 
	out %al, $0x21
	.word 0x00eb, 0x00eb 
	out %al, $0xa1

	# cr0
	mov %cr0, %eax 
	bts $0, %eax 
	mov %eax, %cr0 

	ljmp $8, $0

empty_8042:
	.word 0x00eb, 0x00eb 
	in $0x64, %al 
	test $2, %al 
	jnz empty_8042
	ret 

gdt:
	.word 0,0,0,0

	.word 0x07ff
	.word 0x0000
	.word 0x9a00
	.word 0x00c0 

	.word 0x07ff
	.word 0x0000
	.word 0x9200
	.word 0x00c0

	.quad 0x00c0920b80000002	# video segment

idt_48:
	.word 0
	.word 0,0

gdt_48:
	.word 0x800
	.word 512+gdt, 0x9

msg:
	.byte 13,10
	.ascii "Loading setup"
	.byte 13,10

	.fill 128, 4, 0
stk:

