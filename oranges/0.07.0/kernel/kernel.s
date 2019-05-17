%include "sconst.inc"

extern cstart
extern kernel_main
extern clock_handler

extern exception_handler
extern spurious_irq

extern gdt_ptr
extern idt_ptr
extern disp_pos 
extern proc_ready
extern tss 
extern k_reenter
extern irq_table
extern syscall_table

bits 32
[section .bss]
StackSpace	resb 2*1024
StackTop:

[section .text]
global _start
global restart 
global sys_call

global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
global  hwint00
global  hwint01
global  hwint02
global  hwint03
global  hwint04
global  hwint05
global  hwint06
global  hwint07
global  hwint08
global  hwint09
global  hwint10
global  hwint11
global  hwint12
global  hwint13
global  hwint14
global  hwint15

_start:
	mov esp, StackTop
	mov dword [disp_pos], 0

	sgdt [gdt_ptr]
	call cstart
	lgdt [gdt_ptr]
	lidt [idt_ptr]

	jmp SELECTOR_KERNEL_CS: csinit
csinit:
	
	xor eax, eax 
	mov ax, SELECTOR_TSS
	ltr ax 

	jmp kernel_main

%macro 	hwint_master 1
	call save 

	in al, INT_M_CTLMASK
	or al, 1 << %1
	out INT_M_CTLMASK, al

	mov al, EOI 
	out INT_M_CTL, al 

	sti 
	push %1
	call [irq_table + 4 * %1]
	pop ecx 
	cli 

	in al, INT_M_CTLMASK
	and al, ~(1 << %1)
	out INT_M_CTLMASK, al 

	ret 
%endmacro


align 16
hwint00:		; clock interrupt
	hwint_master 0

align 16
hwint01:
	hwint_master 1

align 16
hwint02:
	hwint_master 2

align 16
hwint03:
	hwint_master 3

align 16
hwint04:
	hwint_master 4

align 16
hwint05:
	hwint_master 5

align 16
hwint06:
	hwint_master 6

align 16
hwint07:
	hwint_master 7

%macro 	hwint_slave 1
	push %1
	call spurious_irq
	add esp, 4
	hlt 
%endmacro

align 16
hwint08:
	hwint_slave 8

align 16
hwint09:
	hwint_slave 9

align 16
hwint10:
	hwint_slave 10

align 16
hwint11:
	hwint_slave 11

align 16
hwint12:
	hwint_slave 12

align 16
hwint13:
	hwint_slave 13

align 16
hwint14:
	hwint_slave 14

align 16
hwint15:
	hwint_slave 15

divide_error:
	push 0xFFFFFFFF
	push 0 
	jmp exception 
single_step_exception:
	push 0xFFFFFFFF
	push 1
	jmp exception
nmi:
	push 0xFFFFFFFF
	push 2
	jmp exception
breakpoint_exception:
	push 0xFFFFFFFF
	push 3
	jmp exception
overflow:
	push 0xFFFFFFFF
	push 4
	jmp exception
bounds_check:
	push 0xFFFFFFFF
	push 5
	jmp exception
inval_opcode:
	push 0xFFFFFFFF
	push 6
	jmp exception
copr_not_available:
	push 0xFFFFFFFF
	push 7
	jmp exception
double_fault:
	push 0xFFFFFFFF
	push 8
	jmp exception
copr_seg_overrun:
	push 0xFFFFFFFF
	push 9
	jmp exception
inval_tss:
	push 10
	jmp exception
segment_not_present:
	push 11
	jmp exception
stack_exception:
	push 12
	jmp exception
general_protection:
	push 13
	jmp exception
page_fault:
	push 14
	jmp exception
copr_error:
	push 0xFFFFFFFF
	push 16
	jmp exception
exception:
	call exception_handler
	add esp, 4*2
	hlt 

save:
	pushad 
	push ds 
	push es 
	push fs 
	push gs 

	mov esi, edx 

	mov dx, ss 
	mov ds, dx 
	mov es, dx 
	mov fs, dx 

	mov edx, esi 

	mov esi, esp

	inc dword [k_reenter]
	cmp dword [k_reenter], 0
	jne .1
	mov esp, StackTop
	push restart
	jmp [esi + RETADR - P_STACKBASE]
.1:
	push restart_reenter
	jmp [esi + RETADR - P_STACKBASE]

sys_call:
	call save
	
	sti
	push esi 
	push dword [proc_ready]
	push edx
	push ecx 
	push ebx 
	call [syscall_table + eax * 4]
	add esp, 4*4
	pop esi 
	mov [esi + EAXREG - P_STACKBASE], eax 
	cli 

	ret 

restart:
	mov esp, [proc_ready]
	lldt [esp + P_LDT_SEL]
	lea eax, [esp + P_STACKTOP]
	mov dword [tss + TSS3_S_SP0], eax 
restart_reenter:
	dec dword [k_reenter]
	pop gs 
	pop fs 
	pop es 
	pop ds 
	popad 

	add esp, 4

	iretd