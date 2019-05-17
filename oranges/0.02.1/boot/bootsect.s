; boot/bootsect.s 2018 by x01

%include "pm.inc"

	org 0x0100
	jmp start
[section .gdt]
gdts:	
gdt_null:	Descriptor 0,0,0
gdt_code32:	Descriptor 0, Code32Len - 1, DA_C + DA_32
gdt_video:	Descriptor 0x0B8000, 0xFFFF, DA_DRW
gdt_normal:	Descriptor 0, 0xFFFF, DA_DRW 
gdt_code16:	Descriptor 0, 0xFFFF, DA_C 
gdt_data:	Descriptor 0, DataLen - 1, DA_DRW 
gdt_stack:	Descriptor 0, TopOfStack, DA_DRWA + DA_32 
gdt_test:	Descriptor 0x500000, 0xFFFF, DA_DRW 

GdtLen equ $ - gdts
gdt_ptr:
	dw	GdtLen - 1
	dd 	0

SelectorCode32	equ gdt_code32 - gdts
SelectorVideo 	equ gdt_video - gdts
SelectorNormal 	equ gdt_normal - gdts
SelectorCode16 	equ gdt_code16 - gdts 
SelectorData 	equ gdt_data - gdts 
selectorStack 	equ gdt_stack - gdts 
SelectorTest 	equ gdt_test - gdts 

[section .start]
[bits 16]
start:
	mov ax, cs
	mov ds, ax 
	mov es, ax 
	mov ss, ax 
	mov sp, 0x0100

	mov [go_back_realmode + 3], ax 
	mov [sp_realmode], sp 

	; 填充基地址
	FillBase seg_code32, gdt_code32
	FillBase seg_code16, gdt_code16
	FillBase seg_data, gdt_data
	FillBase seg_stack, gdt_stack

	; 加载 gdt_ptr
	xor eax, eax
	mov ax, ds 
	shl eax, 4
	add eax, gdts
	mov dword [gdt_ptr + 2], eax 
	
	lgdt	[gdt_ptr]

	; 关中断， 打开 A20, 置 PE 位， 进入保护模式
	cli

	in	al, 0x92
	or 	al, 0x02
	out 0x92, al 

	mov eax, cr0
	or  eax, 1
	mov cr0, eax 

	jmp dword SelectorCode32:0

realmode_entry:
	mov ax, cs 
	mov ds, ax 
	mov es, ax 
	mov ss, ax 
	mov sp, [sp_realmode]

	in al, 0x92
	and al, 0xFD
	out 0x92, al 

	sti 

	mov ax, 0x4C00
	int 0x21

[section .code32]
[bits 32]
seg_code32:
	mov ax, SelectorData
	mov ds, ax 
	mov ax, SelectorTest
	mov es, ax 
	mov ax, SelectorVideo
	mov gs, ax 
	mov ax, selectorStack
	mov ss, ax 
	mov esp, TopOfStack

	mov ah, 0x0C
	xor esi, esi 
	xor edi, edi 
	mov esi, OffsetPmMessage
	mov edi, (80*14)*2
	cld 
.1:
	lodsb
	test al, al 
	jz .2
	mov [gs:edi], ax 
	add edi, 2
	jmp .1
.2:
	call DispReturn
	call TestRead
	call TestWrite
	call TestRead 
	jmp SelectorCode16:0

TestRead:
	xor esi, esi 
	mov ecx, 8
.loop:
	mov al, [es:esi]
	call DispAL
	inc esi 
	loop .loop 
	call DispReturn
	ret 

TestWrite:
	push esi 
	push edi 
	xor esi, esi 
	xor edi, edi 
	mov esi, OffsetTestStr
	cld 
.1:
	lodsb 
	test al, al 
	jz .2
	mov [es:edi], al 
	inc edi 
	jmp .1 
.2:
	pop edi 
	pop esi 
	ret 

DispAL:
	push ecx 
	push edx 
	
	mov ah, 0x0C
	mov dl, al 
	shr al, 4
	mov ecx, 2
.begin:
	and al, 0x0F
	cmp al, 9
	ja .1
	add al, '0'
	jmp .2 
.1:
	sub al, 0x0A
	add al, 'A'
.2:
	mov [gs:edi], ax 
	add edi, 2

	mov al, dl
	loop .begin 
	add edi, 2

	pop edx 
	pop ecx
	ret 

DispReturn:
	push eax 
	push ebx 
	mov eax, edi 
	mov bl, 160
	div bl 
	and eax, 0xFF
	inc eax 
	mov bl, 160
	mul bl 
	mov edi, eax 
	pop ebx 
	pop eax 
	ret 


Code32Len equ $ - seg_code32

section .code16]
align 32
[bits 16]
seg_code16:
	mov ax, SelectorNormal
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax 

	mov eax, cr0 
	and al, 0xFE
	mov cr0, eax 
go_back_realmode:
	jmp 0:realmode_entry
Code16Len equ $ - seg_code16

[section .data]
align 32
[bits 32]
seg_data:
sp_realmode:	dw 0
pm_message:		db "In protect mode", 0
OffsetPmMessage	equ pm_message - $$
test_str:		db "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0
OffsetTestStr 	equ test_str - $$
DataLen 		equ $ - seg_data

[section .stack]
align 32
[bits 32]
seg_stack:	times 512 db 0
TopOfStack	equ $ - seg_stack - 1
