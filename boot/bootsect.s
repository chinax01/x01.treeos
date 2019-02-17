.code16

.equ BOOTSEG, 0x07c0
.equ SYSSEG, 0x1000
.equ SYSSIZE, 0x3000
.equ INITSEG, 0x9000
.equ SETUPSEG, 0x9020
.equ SETUPLEN, 4
.equ ENDSEG, SYSSEG+SYSSIZE

.equ ROOT_DEV, 0x0301
.equ SWAP_DEV, 0x0304

.globl _start
.text

	ljmp $BOOTSEG, $_start
_start:
	mov $BOOTSEG, %ax  
	mov %ax, %ds
	mov $INITSEG, %ax 
	mov %ax, %es
	mov $256, %cx 
	sub %si, %si
	sub %di, %di
	rep 
	movsw 
	ljmp $INITSEG, $go 
go:
	mov %cs, %ax
	mov $0xfef4, %dx 
	mov %ax, %ds 
	mov %ax, %es 
	mov %ax, %ss 
	mov %dx, %sp 

	# 修改软驱参数表
	xor %ax, %ax 
	mov %ax, %ds 
	mov $0x78, %si 
	mov %dx, %di 
	mov $6, %cx 
	cld 
	rep
	movsw
	mov %dx, %di
	movb $18, %es:4(%di)
	mov %di, %ds:0x78
	mov %es, %ds:0x7a
	mov %cs, %ax 
	mov %ax, %ds 
	
	xor %ah, %ah 
	xor %dl, %dl 
	int $0x13
	
load_setup:
	mov $0x0000, %dx		# dh: 磁头号； dl: 驱动器号，0表示A盘
	mov $0x0002, %cx		# ch: 磁道低8位，cl 6-7高2位； cl 0-5： 起始扇区号（从1开始）
	mov $0x0200, %bx 
	mov $0x0200+SETUPLEN, %ax	# ah: 读盘功能号； al： 要读扇区数
	int $0x13
	jnc ok_load
	mov $0x0000, %dx 
	mov $0x0000, %ax 
	int $0x13
	jmp load_setup
ok_load:
	
	# Get drive paramters
	mov $0x00, %dl 
	mov $0x0800, %ax 
	int $0x13
	mov $0x00, %ch 
	mov %cx, %cs:sectors	# cl: sectors
	mov $INITSEG, %ax 
	mov %ax, %es 

	# Print message
	mov $0x03, %ah 
	xor %bh, %bh
	int $0x10

	mov $20, %cx 
	mov $0x000c, %bx 
	mov $msg, %bp 
	mov $0x1301, %ax 
	int $0x10

	# Load system
	mov $SYSSEG, %ax 
	mov %ax, %es 
	call read_it 
	call kill_motor 

	ljmp $SETUPSEG, $0

sread:	.word 1+SETUPLEN
head: 	.word 0
track: 	.word 0

read_it:
	mov %es, %ax 
	test $0x0fff, %ax 
die:
	jne die 
	xor %bx, %bx 
rp_read:
	mov %es, %ax 
	cmp $ENDSEG, %ax 
	jb ok1_read 
	ret 
ok1_read:
	mov %cs:sectors, %ax 
	sub sread, %ax 
	mov %ax, %cx 
	shl $9, %cx 
	add %bx, %cx 
	jnc ok2_read 	# 16位段内偏移，64K
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
	pusha

	#pusha 
	#mov $0x0e2e, %ax 
	#mov $7, %bx 
	#int $0x10
	#popa 
	
	mov track, %dx
	mov sread, %cx 
	inc %cx 		# cl: start_sectnr
	mov %dl, %ch 	# ch: track
	mov head, %dx 
	mov %dl, %dh 	# dh: head
	and $0x0100, %dx 
	mov $2, %ah 
	int $0x13
	jc bad_rt
	popa 
	ret 
bad_rt:
	mov $0, %ax 
	mov $0, %dx 
	int $0x13
	popa 
	jmp read_track

kill_motor:
	push %dx 
	mov $0x3f2, %dx 
	mov $0, %al 
	outsb
	pop %dx 
	ret 

sectors:
	.word 0
msg:
	.byte 13,10
	.ascii "Loading bootsect"
	.byte 13,10

	.org 508
root_dev:
	.word ROOT_DEV
boot_flag:
	.word 0xaa55
