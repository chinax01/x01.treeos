SIG_CHLD = 17
EAX = 0x00
EBX = 0x04
ECX = 0x08
EDX = 0x0C
FS = 0x10
ES = 0x14
DS = 0x18
EIP = 0x1C
CS = 0x20
EFLAGS = 0x24
OLDESP = 0x28
OLDSS = 0x2C 

state = 0
counter = 4
priority = 8
signal = 12
restorer = 16
sig_fn = 20

nr_system_calls = 68 

.globl system_call, sys_fork, timer_interrupt, hd_interrupt, sys_execve, my_call 

.align 2
bad_sys_call:
	movl $-1, %eax 
	iret 
.align 2
reschedule:
	pushl $ret_from_sys_call 
	jmp schedule
.align 2
system_call:
	cmpl $nr_system_calls - 1, %eax 
	ja bad_sys_call
	push %ds 
	push %es 
	push %fs 
	pushl %edx 
	pushl %ecx 
	pushl %ebx 
	movl $0x10, %edx 
	mov %dx, %ds 
	mov %dx, %es 
	movl $0x17, %edx 
	mov %dx, %fs 
	call sys_call_table(, %eax, 4)
	pushl %eax 
	movl current, %eax 
	cmpl $0, state(%eax)
	jne reschedule 
	cmpl $0, counter(%eax)
	je reschedule 
ret_from_sys_call:
	movl current, %eax 
	cmpl task, %eax 
	je 3f 
	movl CS(%esp), %ebx 
	testl $3, %ebx 
	je 3f 
	cmpw $0x17, OLDSS(%esp)
	jne 3f 
2:	movl signal(%eax), %ebx 
	bsfl %ebx, %ecx 	# bit scan forward: 搜索最小的 ‘1’ 位，索引存入目的寄存器
	je 3f 
	btrl %ecx, %ebx 	# bit test and reset
	movl %ebx, signal(%eax) # clear signal bit
	movl sig_fn(%eax, %ecx, 4), %ebx 
	cmpl $1, %ebx 
	jb default_signal 
	je 2b 
	movl $0, sig_fn(%eax, %ecx, 4)
	incl %ecx 
	xchgl %ebx, EIP(%esp)
	subl $28, OLDESP(%esp)
	movl OLDESP(%esp), %edx 
	pushl %eax 
	pushl %ecx 
	pushl $28 
	pushl %edx 
	call verify_area 
	popl %edx 
	addl $4, %esp 
	popl %ecx 
	popl %eax 
	movl restorer(%eax), %eax 
	movl %eax, %fs:(%edx)
	movl %ecx, %fs:4(%edx)
	movl EAX(%esp), %eax 
	movl %eax, %fs:8(%edx)
	movl ECX(%esp), %eax 
	movl %eax, %fs:12(%edx)
	movl EDX(%esp), %eax 
	movl %eax, %fs:16(%edx)
	movl EFLAGS(%esp), %eax 
	movl %eax, %fs:20(%edx)
	movl %ebx, %fs:24(%edx)
3:	popl %eax 
	popl %ebx 
	popl %ecx 
	popl %edx 
	pop %fs 
	pop %es 
	pop %ds 
	iret 

default_signal:
	incl %ecx 
	cmpl $SIG_CHLD, %ecx 
	je 2b 
	pushl %ecx 
	call do_exit 
	addl $4, %esp 
	jmp 3b 

.align 2
timer_interrupt:
	push %ds 
	push %es 
	push %fs 
	pushl %edx 
	pushl %ecx 
	pushl %ebx 
	pushl %eax 
	movl $0x10, %eax 
	mov %ax, %ds 
	mov %ax, %es 
	movl $0x17, %eax 
	mov %ax, %fs 
	incl jiffies 
	movb $0x20, %al 
	outb %al, $0x20 
	movl CS(%esp), %eax 
	andl $3, %eax 
	pushl %eax 
	call do_timer 
	addl $4, %esp 
	jmp ret_from_sys_call

.align 2
sys_execve:
	lea EIP(%esp), %eax 
	pushl %eax 
	call do_execve
	addl $4, %esp 
	ret 

.align 2
sys_fork:
	call find_empty_process 
	testl %eax, %eax 
	js 1f 
	push %gs 
	pushl %esi 
	pushl %edi 
	pushl %ebp 
	pushl %eax 
	call copy_process 
	addl $20, %esp 
1:	ret 

hd_interrupt:
	pushl %eax 
	pushl %ecx 
	pushl %edx 
	push %ds 
	push %es 
	push %fs 
	movl $0x10, %eax 
	mov %ax, %ds 
	mov %ax, %es 
	movl $0x17, %eax 
	mov %ax, %fs 
	movb $0x20, %al 
	outb %al, $0x20 
	jmp 1f 
1:	jmp 1f 
1:	outb %al, $0xa0
	movl do_hd, %eax 
	testl %eax, %eax 
	jne 1f
	movl $unexpected_hd_interrupt, %eax 
1:	call *%eax 
	pop %fs 
	pop %es 
	pop %ds 
	popl %edx 
	popl %ecx 
	popl %eax 
	iret 

my_call:
	pushl %eax 
	pushl %ecx 
	pushl %edx 
	push %ds 
	push %es 
	push %fs 
	movl $0x10, %edx 
	mov %dx, %ds 
	mov %dx, %es 
	movl $0x17, %edx 
	mov %dx, %fs 
	call my_call_table(,%eax, 4)
	pop %fs 
	pop %es 
	pop %ds 
	popl %edx 
	popl %ecx 
	popl %eax
	iret 
	