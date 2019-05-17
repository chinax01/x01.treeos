; boot/bootsect.s 2018 by x01

%include "pm.inc"

PageDirBase0 equ	0x200000
PageTblBase0 equ 	0x201000
PageDirBase1 equ	0x210000
PageTblBase1 equ 	0x211000

LinearAddrDemo	equ 0x00401000
ProcFoo			equ 0x00401000
ProcBar			equ 0x00501000
ProcPagingDemo	equ 0x00301000

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

gdt_pagedir:	Descriptor PageDirBase0, 4095, DA_DRW 
gdt_pagetbl:	Descriptor PageTblBase0, 4096*8-1, DA_DRW

gdt_flatC:		Descriptor 0, 0xFFFF, DA_CR | DA_32 | DA_Limit_4K
gdt_flatD:		Descriptor 0, 0xFFFF, DA_DRW | DA_Limit_4K

GdtLen equ $ - gdts

gdt_ptr:
	dw	GdtLen - 1
	dd 	0

SelectorCode32	equ gdt_code32 - gdts
SelectorVideo 	equ gdt_video - gdts

SelectorNormal 	equ gdt_normal - gdts
SelectorCode16 	equ gdt_code16 - gdts 
SelectorData 	equ gdt_data - gdts 
SelectorStack 	equ gdt_stack - gdts 

SelectorPageDir		equ gdt_pagedir - gdts 
SelectorPageTbl		equ gdt_pagetbl - gdts 

[section .data]
align 32
[bits 32]
seg_data:
sp_realmode:	dw 0
pm_message:		db	"In Protect Mode now. ^-^", 0Ah, 0Ah, 0	 ; db "In protect mode ^-^", 0xA, 0
mem_title:	db "BaseAddrL BaseAddrH LengthLow LengthHigh  Type", 0xA, 0
ram_size:	db "RAM size: ", 0
return:		db 0x0A, 0
mcr_number:	dd 0
disp_pos:	dd (80*6)*2
mem_size:	dd 0
addr_range:
	base_addr_low:	dd 0
	base_addr_high: dd 0
	length_low:		dd 0
	length_high:	dd 0
	type: 			dd 0
mem_buf:	times 256 db 0

OffsetPmMessage	equ pm_message - $$
OffsetMemTitle	equ mem_title - $$
OffsetRamSize	equ ram_size - $$
OffsetReturn 	equ return - $$
OffsetMcrNumber	equ mcr_number - $$
OffsetDispPos	equ disp_pos - $$
OffsetMemSize	equ mem_size - $$
OffsetAddrRange	equ addr_range - $$
	OffsetBaseAddrLow	equ base_addr_low - $$
	OffsetBaseAddrHigh	equ base_addr_high - $$
	OffsetLengthLow		equ length_low - $$
	OffsetLengthHigh 	equ length_high - $$	
	OffsetType 			equ type - $$
OffsetMemBuf	equ mem_buf - $$

DataLen 		equ $ - seg_data

[section .stack]
align 32
[bits 32]
seg_stack:	times 512 db 0
TopOfStack	equ $ - seg_stack - 1

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

	; memory check
	mov ebx, 0
	mov di, mem_buf
.loop:
	mov eax, 0xE820
	mov ecx, 20
	mov edx, 0x534D4150
	int 0x15
	jc check_mem_fail
	add di, 20
	inc dword [mcr_number]
	cmp ebx, 0
	jne .loop
	jmp check_mem_ok
check_mem_fail:
	mov dword [mcr_number], 0
check_mem_ok:

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
	;and al, 0xFD
	and	al, 11111101b
	out 0x92, al 

	sti 

	mov ax, 0x4C00
	int 0x21

[section .code32]
[bits 32]
seg_code32:
	mov ax, SelectorData
	mov ds, ax 
	mov ax, SelectorData
	mov es, ax 
	mov ax, SelectorVideo
	mov gs, ax 
	mov ax, SelectorStack
	mov ss, ax 
	mov esp, TopOfStack

	push OffsetPmMessage
	call DispStr
	add esp, 4

	;push OffsetMemTitle
	;call DispStr 
	;add  esp, 4

	call GetMemSize

	call SetupPaging

	jmp SelectorCode16:0

SetupPaging:
	xor edx, edx
	mov eax, [OffsetMemSize]
	mov ebx, 0x40000
	div ebx 
	mov ecx, eax 
	test edx, edx 
	jz .no_remainder
	inc ecx 
.no_remainder:
	push ecx 

	mov ax, SelectorPageDir
	mov es, ax 
	xor edi, edi 
	xor eax, eax 
	mov eax, PageTblBase0 | PG_P | PG_RW
.1:
	stosd 
	add eax, 4096
	loop .1

	mov ax, SelectorPageTbl
	mov es, ax 
	pop eax 
	mov ebx, 1024
	mul ebx 
	mov ecx, eax 
	xor edi, edi 
	xor eax, eax 
	mov eax, PG_P | PG_RW 
.2:
	stosd 
	add eax, 4096
	loop .2

	mov eax, PageDirBase0
	mov cr3, eax 
	mov eax, cr0 
	or eax, 0x80000000
	mov cr0, eax 
	jmp short .3
.3:
	nop

	ret 

GetMemSize:
	push esi 
	push edi
	push ecx 

	mov esi, OffsetMemBuf
	mov ecx, [OffsetMcrNumber]
.loop:
	mov edx, 5
	mov edi, OffsetAddrRange
.1:
	;push dword [esi]
	;call DispInt
	;pop eax 
	mov eax, dword [esi]
	stosd 
	add esi, 4
	dec edx 
	cmp edx, 0
	jnz .1
	;call DispReturn 
	cmp dword [OffsetType], 1
	jne .2 
	mov eax, [OffsetBaseAddrLow]
	add eax, [OffsetLengthLow]
	cmp eax, [OffsetMemSize]
	jb .2
	mov [OffsetMemSize], eax 
.2:
	loop .loop

	;call DispReturn
	;push OffsetRamSize
	;call DispStr
	;add esp, 4

	push dword [OffsetMemSize]
	call DispInt 
	add esp, 4

	pop ecx 
	pop edi 
	pop esi 
	ret 

%include "lib.inc"

Code32Len equ $ - seg_code32

[section .code16]
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
	and eax, 0x7FFFFFFE
	mov cr0, eax 

go_back_realmode:
	jmp 0:realmode_entry

Code16Len equ $ - seg_code16




