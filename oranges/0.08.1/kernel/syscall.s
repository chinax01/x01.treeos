%include "sconst.inc"

INT_VECTOR_SYS_CALL		equ 0x90
_NR_sendrec				equ 0
_NR_printx				equ 1

global sendrec 
global printx 

bits 32
[section .text]
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

	