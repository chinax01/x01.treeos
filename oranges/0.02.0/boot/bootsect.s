; boot/bootsect.s 2018 by x01

%include "pm.inc"

	org 0x0100
	jmp start
[section .gdt]
gdt_desc:	
gdt_null:	Descriptor 0,0,0
gdt_code32:	Descriptor 0, Code32Len - 1, DA_C + DA_32
gdt_video:	Descriptor 0x0B8000, 0xFFFF, DA_DRW

GdtLen equ $ - gdt_desc
gdt_ptr:
	dw	GdtLen - 1
	dd 	0
SelectorCode32	equ gdt_code32 - gdt_desc
SelectorVideo equ gdt_video - gdt_desc

[section .code16]
[bits 16]
start:
	mov ax, cs
	mov ds, ax 
	mov es, ax 
	mov ss, ax 
	mov sp, 0x0100

	xor eax, eax 
	mov ax, cs 
	shl eax, 4
	add eax, seg_code32
	mov word [gdt_code32 + 2], ax 
	shr eax, 16
	mov byte [gdt_code32 + 4], al
	mov byte [gdt_code32 + 7], ah

	xor eax, eax
	mov ax, ds 
	shl eax, 4
	add eax, gdt_desc
	mov dword [gdt_ptr + 2], eax 
	
	lgdt	[gdt_ptr]

	cli

	in	al, 0x92
	or 	al, 0x02
	out 0x92, al 

	mov eax, cr0
	or  eax, 1
	mov cr0, eax 

	jmp dword SelectorCode32:0

[section .code32]
[bits 32]
seg_code32:
	mov ax, SelectorVideo
	mov gs, ax 

	mov edi, (80*14) * 2
	mov ah, 0x0C
	mov al, 'P'
	mov [gs:edi], ax 

	jmp $

Code32Len equ $ - seg_code32
