# boot/boot.S
.code16

.globl start

.equ SysSize,	0x3000
.equ BootSeg,	0x07c0
.equ InitSeg,	0x9000
.equ SysSeg,	0x1000
.equ EndSeg,	SysSeg+SysSize

	# load boot sector to InitSeg and continue...
	ljmp $BootSeg, $start
start:
	mov $BootSeg, %ax
	mov %ax, %ds 
	mov $InitSeg, %ax 
	mov %ax, %es 
	mov $256, %cx 
	sub %si, %si 
	sub %di, %di 
	rep 
	movsw
	ljmp $InitSeg, $go 
go:
	mov %cs, %ax 
	mov %ax, %ds 
	mov %ax, %es 
	mov %ax, %ss 
	mov $0xff00, %sp 

	# get drive parameters
	mov $0x00, %dl 		# dl=0 软盘
	mov $0x0800, %ax 	# ah=8, 获取驱动器参数
	int $0x13
	mov $0x00, %ch 		# cl 0-5位：扇区数
	mov %cx, %cs:sectors 	
	mov $InitSeg, %ax 
	mov %ax, %es 

	# clear screen
	mov $0x0600, %ax 	# ah=0x06 向上滚屏， al=0 清屏
	mov $0x0700, %bx 	# bh=0x07 空白区域属性
	mov $0, %cx 		# ch=y, cl=x 左上角坐标
	mov $0x184f, %dx 	# dh=y, dl=x 右下角坐标
	int $0x10

	# print message
	mov $0, %dx 		# dh=y, dl=x 坐标
	mov $17, %cx 		# 字符串长度
	mov $0x000c, %bx	# bh 页码， bl 属性
	mov $msg, %bp		# es:bp 显示字符串地址
	mov $0x1301, %ax 	# ah=0x13 teletype 显示字符串， al=1 显示后光标改变
	int $0x10

	# load system
	mov $SysSeg, %ax 
	mov %ax, %es 
	call read_it	# 读 system 到 es:bx
	call kill_motor

	# save cursor pos
	mov $0x03, %ah 
	xor %bh, %bh 
	int $0x10
	mov %dx, %ds:0	# 0x90000

	# protected mode
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
	mov %cs, %ax 
	mov %ax, %ds 
	lidt idt_48
	lgdt gdt_48

	# A20
	inb $0x92, %al 
	orb $0x02, %al 
	outb %al, $0x92

	# 8259
	mov $0x11, %al 	# ICW4 needed, cascade mode, level-triggered
	out %al, $0x20
	.word 0x00eb, 0x00eb 
	out %al, $0xa0
	.word 0x00eb, 0x00eb

	mov $0x20, %al 	# 0x20-27
	out %al, $0x21 
	.word 0x00eb, 0x00eb 
	mov $0x28, %al 	# 0x28-2f
	out %al, $0xa1
	.word 0x00eb, 0x00eb 	

	mov $0x04, %al 	# 8259a-1 master
	out %al, $0x21
	.word 0x00eb, 0x00eb
	mov $0x02, %al 	# 8259a-2 slave
	out %al, $0xa1
	.word 0x00eb, 0x00eb

	mov $0x01, %al	# 8086 mode
	out %al, $0x21
	.word 0x00eb, 0x00eb 
	out %al, $0xa1 
	.word 0x00eb, 0x00eb

	mov $0xff, %al 
	out %al, $0x21
	.word 0x00eb, 0x00eb 
	out %al, $0xa1

	mov %cr0, %eax 
	bts $0, %eax 	# set PE bit
	mov %eax, %cr0

	jmp $8, $0		# jmp head 8:0

sectors: .word 0
sread:	 .word 1
head:	 .word 0
track: 	 .word 0

read_it:
	mov %es, %ax 
	test $0x0fff, %ax	# 段对齐，ZF置位
die:
	jne die 
	xor %bx, %bx 
rp_read:
	mov %es, %ax 
	cmp $EndSeg, %ax 
	jb ok1_read		# jb: 小于跳转
	ret 
ok1_read:
	mov %cs:sectors, %ax 
	sub sread, %ax 	 # al: 每磁道所读扇区数
	mov %ax, %cx 
	shl $9, %cx 
	add %bx, %cx 
	jnc ok2_read 	# 每次读16位， 0x1000<<4
	je ok2_read 
	xor %ax, %ax
	sub %bx, %ax 
	shr $9, %ax 
ok2_read:
	call read_track
	mov %ax, %cx 
	add sread, %ax 
	cmp %cs:sectors, %ax 
	jne ok3_read 
	mov $1, %ax 
	sub head, %ax 
	jne ok4_read 
	incw track 
ok4_read:
	mov %ax, head 
	xor %ax, %ax 
ok3_read:
	mov %ax, sread 
	shl $9, %cx 
	add %cx, %bx 
	jnc rp_read
	mov %es, %ax 
	add $0x1000, %ax 
	mov %ax, %es 
	xor %bx, %bx 
	jmp rp_read

read_track:
	push %ax 
	push %bx 
	push %cx 
	push %dx 
	mov track, %dx 
	mov sread, %cx 
	inc %cx 		# cl: 开始读扇区
	mov %dl, %ch 	# ch: 当前磁道号
	mov head, %dx 	
	mov %dl, %dh 	# dh: 磁头号
	mov $0, %dl 	# dl: 0表示A驱动器
	and $0x0100, %dx  # 磁头不大于1
	mov $2, %ah 	# ah=2: 读磁盘功能号; al: 扇区数
	int $0x13		# 读到 es:bx 
	jc bad_rt
	pop %dx 
	pop %cx 
	pop %bx 
	pop %ax 
	ret 
bad_rt:
	mov $0, %ax
	mov $0, %dx 
	int $0x13
	pop %dx 
	pop %cx 
	pop %bx
	pop %ax 
	jmp read_track

kill_motor:
	push %dx 
	mov $0x3f2, %dx 
	mov $0, %al 
	outsb 
	pop %dx 
	ret 

gdt:
	.word 0,0,0,0	# null

	.word 0x07ff	# limit 8M
	.word 0x0000	# base address 0
	.word 0x9a00	# code read/exec
	.word 0x00c0	# granularity=4K, 386

	.word 0x07ff
	.word 0x0000
	.word 0x9200	# data read/write
	.word 0x00c0

gdt_48:
	.word 0x800		# limit 256 entries
	.word gdt, 0x9	# gdt base = 0x9XXXX

idt_48:
	.word 0
	.word 0,0

msg:
	.byte 13,10
	.ascii "hello x01.os!"
	.byte 13,10

	.org 510
	.word 0xaa55