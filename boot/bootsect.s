.code16

.globl _start
.text

.equ BOOTSEG, 0x07c0
.equ SYSSEG, 0x1000
.equ SYSLEN, 16

	ljmp $BOOTSEG, $_start
_start:
	mov $BOOTSEG, %ax  
	mov %ax, %ds
	mov %ax, %ss 
	mov $0x400, %sp

load_system:
	mov $0x0000, %dx		# dh: 磁头号； dl: 驱动器号，0表示A盘
	mov $0x0002, %cx		# ch: 磁道低8位，cl 6-7高2位； cl 0-5： 起始扇区号（从1开始）
	mov $SYSSEG, %ax		# 数据读入 %es:%bx
	mov %ax, %es 
	xor %bx, %bx 				
	mov $0x200+SYSLEN, %ax	# ah: 读盘功能号； al： 要读扇区数
	int $0x13
	jnc ok_load
die:
	jmp die 
ok_load:

	# 移动到内存 0 处
	cli 
	mov $SYSSEG, %ax 
	mov %ax, %ds 
	xor %ax, %ax 
	mov %ax, %es 
	mov $0x1000, %cx		# 8k: SYSLEN*512 bytes
	sub %si, %si 
	sub %di, %di 
	rep 
	movsw 		# %ds:%si =>%es:%di 

	mov $BOOTSEG, %ax 
	mov %ax, %ds 
	lidt idt_48
	lgdt gdt_48

	# 进入保护模式
	mov $0x0001, %ax
	lmsw %ax 

	ljmp $8, $0	 # 跳入代码段(基地址0），即 head 处开始执行

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

idt_48:
	.word 0
	.word 0, 0

gdt_48:
	.word 0x7ff
	.word 0x7c00+gdt, 0

	.org 510
	.word 0xaa55
