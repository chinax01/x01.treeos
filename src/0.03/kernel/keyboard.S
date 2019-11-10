.text
.globl keyboard_interrupt

size = 1024		# 键盘缓冲区的长度，与 tty_io.c 匹配

# tty_queue 结构中的偏移
head = 4
tail = 8
proc_list = 12
buf = 16

mode: .byte 0	# 特殊键状态 位0-1 左右shift; 2-3 左右ctrl; 4-5 左右alt; 6 caps状态， 7 caps按下
leds: .byte 2	# 指示灯状态 位0 scroll-lock; 位1 num-lock; 位2 caps-lock
e0: .byte 0		# 扫描前导码

keyboard_interrupt:
	pushl %eax 
	pushl %ebx 
	pushl %ecx 
	pushl %edx 
	push %ds 
	push %es 
	movl $0x10, %eax 
	mov %ax, %ds 
	mov %ax, %es 
	xor %al, %al 
	inb $0x60, %al 		# 读取扫描码
	pushl %eax 
	movzx %al, %eax 
	movl %eax, keybuf 	# 在 shell.c
	popl %eax 
	cmpb $0xe0, %al 
	je set_e0
	cmpb $0xe1, %al 
	je set_e1
	call *key_table(,%eax,4)
	movb $0, e0 
e0_e1:
	inb $0x61, %al 	# 0x61: 8255A 输出端口位7允许禁止键盘工作
	jmp 1f
1:	jmp 1f
1:	orb $0x80, %al # 禁止
	jmp 1f
1: 	jmp 1f
1: 	outb %al, $0x61
	jmp 1f
1:	jmp 1f
1:	andb $0x7f, %al # 允许
	outb %al, $0x61 

	movb $0x20, %al # 向 8259 发送 EOI
	outb %al, $0x20 

	pushl $0 	# tty=0
	call do_tty_interrupt
	call os_shell 
	addl $4, %esp 
	pop %es 
	pop %ds 
	popl %edx 
	popl %ecx 
	popl %ebx 
	popl %eax 
	iret 
set_e0:
	movb $1, e0 
	jmp e0_e1
set_e1:
	movb $2, e0 
	jmp e0_e1

# 把 ebx:eax 最多8个字符添如缓冲队列
# 1. 从 table_list 取 read_q 地址，al 中字符复制到头指针， 前移1字节位置。
# 2. ebx:eax 右移8位，重复对 al 的处理
put_queue:
	pushl %ecx 
	pushl %edx 
	movl table_list, %edx 
	movl head(%edx), %ecx 
1:	movb %al, buf(%edx, %ecx)
	incl %ecx 
	andl $size-1, %ecx 		# 回绕
	cmpl tail(%edx), %ecx 	# 已满
	je 3f 
	shrdl $8, %ebx, %eax 	# ebx 右移8位到 eax， ebx 不变
	je 2f 					# 为 0 没有字符时跳转
	shrl $8, %ebx 			# ebx 右移8位
	jmp 1b 
2:	movl %ecx, head(%edx)
	movl proc_list(%edx), %ecx 
	testl %ecx, %ecx 
	je 3f 
	movl $0, (%ecx)	# 置等待进程就绪状态
3:	popl %edx 
	popl %ecx 
	ret 

do_self:
	lea alt_map, %ebx 
	testb $0x20, mode 	# 位与结果为0时 ZF=1
	jne 1f 				# 结果不为0(ZF=0)时跳转 
	lea shift_map, %ebx 
	testb $0x03, mode 
	jne 1f 
	lea key_map, %ebx 
1:	movb (%ebx, %eax), %al 
	orb %al, %al 	
	je none				# 结果为0 跳转 
	testb $0x4c, mode 	# ctrl or caps
	je 2f 
	cmpb $'a, %al 
	jb 2f 
	cmpb $'z, %al 
	ja 2f 
	subb $32, %al 
2:	testb $0x0c, mode 	# ctrl
	je 3f 
	cmpb $64, %al 
	jb 3f 
	cmpb $64+32, %al 
	jae 3f 
	subb $64, %al 		# 字符'@-_' 到控制字符 0-31
3:	testb $0x10, mode 	# left alt
	je 4f 
	orb $0x80, %al 
4:	andl $0xff, %eax 
	xorl %ebx, %ebx 
	call put_queue		# 将字符放入缓冲队列
none:
	ret 

minus:
	cmpb $1, e0 
	jne do_self
	movl $'/, %eax 
	xorl %ebx, %ebx 
	jmp put_queue

ctrl:
	movb $0x04, %al 
	jmp 1f 
alt:
	movb $0x10, %al 
1:	cmpb $0, e0 	# e0 置位 right-side
	je 2f 
	addb %al, %al 
2:	orb %al, mode	# 置位 mode
	ret 

unctrl:
	movb $0x04, %al 
	jmp 1f 
unalt:
	movb $0x10, %al 
1:	cmpb $0, e0 
	je 2f 
	addb %al, %al 
2:	notb %al 
	andb %al, mode 	# 复位 mode
	ret 

lshift:
	orb $0x01, mode 
	ret 
unlshift:
	andb $0xfe, mode 
	ret 
rshift:
	orb $0x02, mode 
	ret 
unrshift:
	andb $0xfd, mode 
	ret 

caps:
	testb $0x80, mode 
	jne 1f 
	xorb $4, leds 		# 翻转位2
	xorb $0x40, mode 
	orb $0x80, mode 
set_leds:
	call kb_wait
	movb $0xed, %al 
	outb %al, $0x60 	# 键盘命令 0xed 到 0x60
	call kb_wait 
	movb leds, %al 		
	outb %al, $0x60 
	ret 

uncaps:
	andb $0x7f, mode 
	ret 

scroll:
	xorb $1, leds 
	jmp set_leds 
num:
	xorb $2, leds 
	jmp set_leds 

cursor:
	subb $0x47, %al 	# 0x47-0x53: 数字小键盘
	jb 1f 
	cmpb $12, %al 
	ja 1f 
	jne cur2 
	testb $0x0c, mode 
	je cur2 
	testb $0x30, mode 
	jne reboot 			# ctl+alt+del
cur2:
	cmpb $0x01, e0 		# e0 置位作方向键
	je cur 
	testb $0x02, leds 	# num-lock 没置位作方向键
	je cur 
	testb $0x03, mode 	# shift 按下作方向键
	jne cur 

	xorl %ebx, %ebx 
	movb num_table(%eax), %al 	# 取数字字符
	jmp put_queue
1:	ret 

cur:
	movb cur_table(%eax), %al 
	cmpb $'9, %al 
	ja ok_cur 
	movb $'~, %ah 
ok_cur:
	shll $16, %eax 
	movw $0x5b1b, %ax 
	xorl %ebx, %ebx 
	jmp put_queue

num_table:
	.ascii "789 456 1230,"
cur_table:
	.ascii "HA5 DGC YB623"

func:
	subb $0x3b, %al 
	jb end_func 
	cmpb $9, %al 
	jbe ok_func 
	subb $18, %al 
	cmpb $10, %al 
	jb end_func
	cmpb $11, %al 
	ja end_func
ok_func:
	cmpl $4, %ecx 
	jl end_func			# jl: 小于跳转
	movl func_table(,%eax,4), %eax 
	xorl %ebx, %ebx 
	jmp put_queue
end_func:
	ret 

func_table:
	.long 0x415b5b1b,0x425b5b1b,0x435b5b1b,0x445b5b1b
	.long 0x455b5b1b,0x465b5b1b,0x475b5b1b,0x485b5b1b
	.long 0x495b5b1b,0x4a5b5b1b,0x4b5b5b1b,0x4c5b5b1b

kb_wait:
	pushl %eax 
1:	inb $0x64, %al 
	testb $0x02, %al 
	jne 1b 
	popl %eax 
	ret 

reboot:
	call kb_wait
	movw $0x1234, 0x472
	movb $0xfc, %al 
	outb %al, $0x64 
die:
	jmp die 

os_shell:
	ret 
	
key_map:
	.byte 0, 27				# NUL, ESC
	.ascii "1234567890-="
	.byte 127, 9			# DEL, TAB
	.ascii "qwertyuiop[]"
	.byte 13, 0				# CR, NUL
	.ascii "asdfghjkl;'"
	.byte '`, 0
	.ascii "\\zxcvbnm,./"
	.byte 0, '*, 0, 32		# 32:SPACE
	.fill 16, 1, 0
	.byte '-, 0, 0, 0, '+
	.byte 0, 0, 0, 0, 0, 0, 0
	.byte '<
	.fill 10, 1, 0

shift_map:
	.byte 0, 27
	.ascii "!@#$%^&*()_+"
	.byte 127,9
	.ascii "QWERTYUIOP{}"
	.byte 13,0
	.ascii "ASDFGHJKL:\""
	.byte '~, 0
	.ascii "|ZXCVBNM<>?"
	.byte 0, '*, 0, 32
	.fill 16, 1, 0
	.byte '-,0,0,0,'+
	.byte 0,0,0,0,0,0,0
	.byte '>
	.fill 10, 1, 0

alt_map:
	.byte 0, 0
	.ascii "\0@\0$\0\0{[]}\\\0"
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte '~,13,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0,0,0
	.fill 16,1,0
	.byte 0,0,0,0,0
	.byte 0,0,0,0,0,0,0
	.byte '|
	.fill 10,1,0

key_table:
	.long none,do_self,do_self,do_self		/* 00-03 s0 esc 1 2 */
	.long do_self,do_self,do_self,do_self	/* 04-07 3 4 5 6 */
	.long do_self,do_self,do_self,do_self	/* 08-0B 7 8 9 0 */
	.long do_self,do_self,do_self,do_self	/* 0C-0F + ' bs tab */
	.long do_self,do_self,do_self,do_self	/* 10-13 q w e r */
	.long do_self,do_self,do_self,do_self	/* 14-17 t y u i */
	.long do_self,do_self,do_self,do_self	/* 18-1B o p } ^ */
	.long do_self,ctrl,do_self,do_self		/* 1C-1F enter ctrl a s */
	.long do_self,do_self,do_self,do_self	/* 20-23 d f g h */
	.long do_self,do_self,do_self,do_self	/* 24-27 j k l | */
	.long do_self,do_self,lshift,do_self	/* 28-2B { para lshift , */
	.long do_self,do_self,do_self,do_self	/* 2C-2F z x c v */
	.long do_self,do_self,do_self,do_self	/* 30-33 b n m , */
	.long do_self,minus,rshift,do_self		/* 34-37 . - rshift * */
	.long alt,do_self,caps,func				/* 38-3B alt sp caps f1 */
	.long func,func,func,func				/* 3C-3F f2 f3 f4 f5 */
	.long func,func,func,func				/* 40-43 f6 f7 f8 f9 */
	.long func,num,scroll,cursor			/* 44-47 f10 num scr home */
	.long cursor,cursor,do_self,cursor		/* 48-4B up pgup - left */
	.long cursor,cursor,do_self,cursor		/* 4C-4F n5 right + end */
	.long cursor,cursor,cursor,cursor		/* 50-53 dn pgdn ins del */
	.long none,none,do_self,func			/* 54-57 sysreq ? < f11 */
	.long func,none,none,none				/* 58-5B f12 ? ? ? */
	.long none,none,none,none				/* 5C-5F ? ? ? ? */
	.long none,none,none,none				/* 60-63 ? ? ? ? */
	.long none,none,none,none		/* 64-67 ? ? ? ? */
	.long none,none,none,none		/* 68-6B ? ? ? ? */
	.long none,none,none,none		/* 6C-6F ? ? ? ? */
	.long none,none,none,none		/* 70-73 ? ? ? ? */
	.long none,none,none,none		/* 74-77 ? ? ? ? */
	.long none,none,none,none		/* 78-7B ? ? ? ? */
	.long none,none,none,none		/* 7C-7F ? ? ? ? */
	.long none,none,none,none		/* 80-83 ? br br br */
	.long none,none,none,none		/* 84-87 br br br br */
	.long none,none,none,none		/* 88-8B br br br br */
	.long none,none,none,none		/* 8C-8F br br br br */
	.long none,none,none,none		/* 90-93 br br br br */
	.long none,none,none,none		/* 94-97 br br br br */
	.long none,none,none,none		/* 98-9B br br br br */
	.long none,unctrl,none,none		/* 9C-9F br unctrl br br */
	.long none,none,none,none		/* A0-A3 br br br br */
	.long none,none,none,none		/* A4-A7 br br br br */
	.long none,none,unlshift,none	/* A8-AB br br unlshift br */
	.long none,none,none,none		/* AC-AF br br br br */
	.long none,none,none,none		/* B0-B3 br br br br */
	.long none,none,unrshift,none	/* B4-B7 br br unrshift br */
	.long unalt,none,uncaps,none	/* B8-BB unalt br uncaps br */
	.long none,none,none,none		/* BC-BF br br br br */
	.long none,none,none,none		/* C0-C3 br br br br */
	.long none,none,none,none		/* C4-C7 br br br br */
	.long none,none,none,none		/* C8-CB br br br br */
	.long none,none,none,none		/* CC-CF br br br br */
	.long none,none,none,none		/* D0-D3 br br br br */
	.long none,none,none,none		/* D4-D7 br br br br */
	.long none,none,none,none		/* D8-DB br ? ? ? */
	.long none,none,none,none		/* DC-DF ? ? ? ? */
	.long none,none,none,none		/* E0-E3 e0 e1 ? ? */
	.long none,none,none,none		/* E4-E7 ? ? ? ? */
	.long none,none,none,none		/* E8-EB ? ? ? ? */
	.long none,none,none,none		/* EC-EF ? ? ? ? */
	.long none,none,none,none		/* F0-F3 ? ? ? ? */
	.long none,none,none,none		/* F4-F7 ? ? ? ? */
	.long none,none,none,none		/* F8-FB ? ? ? ? */
	.long none,none,none,none		/* FC-FF ? ? ? ? */




