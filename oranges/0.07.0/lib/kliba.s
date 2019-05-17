%include "sconst.inc"

extern disp_pos

[section .text]
global strcpy 
global strlen 
global memcpy
global memset 
global disp_str
global disp_color_str
global out_byte 
global in_byte 
global enable_irq
global disable_irq
global disable_int
global enable_int

disable_int:
	cli 
	ret 

enable_int:
	sti 
	ret 

; void disable_irq(int irq);
disable_irq:
	mov ecx, [esp + 4]
	pushf 
	cli 
	mov ah, 1
	rol ah, cl 
	cmp cl, 8
	jae disable_8
disable_0:
	in al, INT_M_CTLMASK
	test al, ah 
	jnz dis_already
	or al, ah 
	out INT_M_CTLMASK, al 
	popf 
	mov eax, 1
	ret
disable_8:
	in al, INT_S_CTLMASK
	test al, ah 
	jnz dis_already
	or al, ah 
	out INT_S_CTLMASK, al 
	popf 
	mov eax, 1
	ret 
dis_already:
	popf 
	xor eax, eax 
	ret 

; void enable_irq(int irq);
enable_irq:
	mov ecx, [esp+4]
	pushf 
	cli 
	mov ah, ~1
	rol ah, cl 
	cmp cl, 8
	jae enable_8
enable_0:
	in al, INT_M_CTLMASK
	and al, ah 
	out INT_M_CTLMASK, al 
	popf 
	ret 
enable_8:
	in al, INT_S_CTLMASK
	and al, ah 
	out INT_S_CTLMASK, al 
	popf 
	ret 

; char* strcpy(char* p_dst, char* p_src);
strcpy:
	push    ebp
	mov     ebp, esp

	mov     esi, [ebp + 12] ; Source
	mov     edi, [ebp + 8]  ; Destination

.1:
	mov     al, [esi]               ; ┓
	inc     esi                     ; ┃
					; ┣ 逐字节移动
	mov     byte [edi], al          ; ┃
	inc     edi                     ; ┛

	cmp     al, 0           ; 是否遇到 '\0'
	jnz     .1              ; 没遇到就继续循环，遇到就结束

	mov     eax, [ebp + 8]  ; 返回值

	pop     ebp
	ret                     ; 函数结束，返回

; void* memcpy(void* es:dest, void* ds:src, int size);
memcpy:
	push ebp
	mov ebp, esp 
	
	push esi 
	push edi 
	push ecx 

	mov edi, [ebp+8]
	mov esi, [ebp+12]
	mov ecx, [ebp+16]
.1:
	cmp ecx, 0
	jz .2

	mov al, [ds:esi]
	inc esi 

	mov byte [es:edi], al 
	inc edi 

	dec ecx 
	jmp .1
.2:
	mov eax, [ebp + 8]

	pop ecx 
	pop edi 
	pop esi 
	mov esp, ebp
	pop ebp 

	ret 

; void memset(void* p_dst, char ch, int size);
memset:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	edx, [ebp + 12]	; Char to be putted
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	byte [edi], dl
	inc	edi			

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回

; void disp_str(char* s);
disp_str:
	push ebp 
	mov ebp, esp 
	push ebx 
	push esi 
	push edi 

	mov esi, [ebp + 8]
	mov edi, [disp_pos]
	mov ah, 0x0F
.1:
	lodsb
	test al, al 
	jz .2
	cmp al, 0x0A
	jnz .3 
	push eax 
	mov eax, edi 
	mov bl, 160
	div bl 
	and eax, 0xFF
	inc eax 
	mov bl, 160
	mul bl 
	mov edi, eax 
	pop eax 
	jmp .1
.3:
	mov [gs:edi], ax 
	add edi, 2
	jmp .1
.2:
	mov [disp_pos], edi 
	
	pop edi 
	pop esi 
	pop ebx 
	pop ebp

	ret

; void disp_color_str(char* s, int color);
disp_color_str:
	push ebp
	mov ebp, esp 

	mov esi, [ebp+8]
	mov edi, [disp_pos]
	mov ah, [ebp+12]
.1:
	lodsb
	test al, al
	jz .2
	cmp al, 0x0A
	jnz .3
	push eax 
	mov eax, edi
	mov bl, 160
	div bl 
	and eax, 0xFF
	inc eax 
	mov bl, 160
	mul bl 
	mov edi, eax 
	pop eax 
	jmp .1
.3:
	mov [gs:edi], ax 
	add edi, 2
	jmp .1
.2:
	mov [disp_pos], edi 
	
	pop ebp 
	ret 

; void out_byte(unsigned short port, unsigned char value);
out_byte:
	mov edx, [esp+4]
	mov al, [esp+8]
	out dx, al 
	nop
	nop
	ret 

; unsigned char in_byte(unsigned short port);
in_byte:
	mov edx, [esp+4]
	xor eax, eax 
	in al, dx 
	nop
	nop
	ret 

; int strlen(char* p_str);
strlen:
        push    ebp
        mov     ebp, esp

        mov     eax, 0                  ; 字符串长度开始是 0
        mov     esi, [ebp + 8]          ; esi 指向首地址

.1:
        cmp     byte [esi], 0           ; 看 esi 指向的字符是否是 '\0'
        jz      .2                      ; 如果是 '\0'，程序结束
        inc     esi                     ; 如果不是 '\0'，esi 指向下一个字符
        inc     eax                     ;         并且，eax 自加一
        jmp     .1                      ; 如此循环

.2:
        pop     ebp
        ret                             ; 函数结束，返回
	

