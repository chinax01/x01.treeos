; DA: Descriptor Attribute
DA_32 			equ 0x4000
DA_LIMIT_4K		equ 0x8000

DA_DPL0 equ 0x00
DA_DPL1 equ 0x20
DA_DPL2 equ 0x40
DA_DPL3 equ 0x60

DA_DR 	equ 0x90
DA_DRW 	equ 0x92
DA_DRWA equ 0x93
DA_C 	equ 0x98
DA_CR 	equ 0x9A
DA_CCO 	equ 0x9C
DA_CCOR equ 0x9E

DA_LDT 		equ 0x82
DA_TaskGate equ 0x85
DA_386TSS 	equ 0x89
DA_386CGate equ 0x8C
DA_386IGate equ 0x8E
DA_386TGate equ 0x8F

; SA: Selector Attribute
SA_TIG		equ 0
SA_TIL		equ 4

SA_RPL0		equ 0
SA_RPL1 	equ 1
SA_RPL2		equ 2
SA_RPL3		equ 3


PG_P		equ 1
PG_RWW		equ 2
PG_USU		equ 0
PG_S		equ 4

; Descriptor base, limit, attr
%macro Descriptor 3
	dw %2 & 0xFFFF
	dw %1 & 0xFFFF
	db (%1 >> 16) & 0xFF
	dw ((%2 >> 8) & 0x0F00) | (%3 & 0xF0FF)
	db (%1 >> 24) & 0xFF
%endmacro

; FillBase baseAddr, gdt 
%macro FillBase 2
	xor eax, eax 
	mov ax, cs 
	shl eax, 4
	add eax, %1
	mov word [%2 + 2], ax 
	shr eax, 16
	mov byte [%2 + 4], al
	mov byte [%2 + 7], ah
%endmacro

; Gate selector, offset, dcount, attr 
%macro Gate 4
	dw %2 & 0xFFFF
	dw %1 & 0xFFFF
	dw (%3 & 0x1F) | ((%4 << 8) & 0xFF00)
	dw ((%2 >> 16) & 0xFFFF)
%endmacro