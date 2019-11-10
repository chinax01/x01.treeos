# boot/boot.S
.code16

.equ SysSize,	0x3000
.equ BootSeg,	0x07c0
.equ InitSeg,	0x9000
.equ SysSeg,	0x1000
.equ EndSeg,	SysSeg+SysSize

	# load boot sector to InitSeg and continue...
	ljmp $BootSeg, $_start
_start:
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

die:
	jmp die 
	
msg:
	.byte 13,10
	.ascii "hello x01.os!"
	.byte 13,10

	.org 510
	.word 0xaa55