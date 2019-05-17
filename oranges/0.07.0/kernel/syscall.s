%include "sconst.inc"

_NR_get_ticks			equ 0
_NR_write				equ 1
_NR_sendrec				equ 2
_NR_printx				equ 3

INT_VECTOR_SYS_CALL		equ 0x90

global get_ticks
global write 
global sendrec 

bits 32
[section .text]
get_ticks:
	mov eax, _NR_get_ticks
	int INT_VECTOR_SYS_CALL
	ret 

write:
	mov eax, _NR_write
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	ret

; int sendrec(int func, int src_dest, struct Message* m);
sendrec:
	mov eax, _NR_sendrec
	mov ebx, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	int INT_VECTOR_SYS_CALL
	ret 
	
; void printx(char* s);
printx:
	mov eax, _NR_printx
	mov edx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret 

	