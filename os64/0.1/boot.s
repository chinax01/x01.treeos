; boot.s

	org 0x7c00
BaseOfStack equ 0x7c00

_start:
	mov ax, cs 
	mov ds, ax 
	mov es, ax 
	mov ss, ax 
	mov sp, BaseOfStack

	; clear screen 
	; int 0x10
	;	ah=6（向上滚屏) al(滚动行数，0清屏)
	;	bh(空白区域缺省值)
	;	ch,cl(左上角x,y) dh,dl(右下角x,y)
	mov ax, 0x0600
	mov bx, 0x0700
	mov cx, 0
	mov dx, 0x184f
	int 0x10

	; set focus
	; int 0x10
	;	ah=2(设置光标位置) bh(显示页码)
	;	dh,dl(行y,列x)
	mov ax, 0x0200
	mov bx, 0x0000
	mov dx, 0x0000
	int 0x10

	; show message
	; int 0x10:
	;	ah=0x13(在teletype模式下显示字符串)
	;	al=1(只含字符，属性在bl中，光标位置改变)
	;	bh(页码) bl(属性)
	;	cx(字符数量)
	;	dh,dl(坐标行,列)
	;	es:bp(显示地址)
	mov ax, 0x1301
	mov bx, 0x000f
	mov dx, 0x0000
	mov cx, 11
	push ax 
	mov ax, ds 
	mov es, ax 
	pop ax 
	mov bp, BootMessage
	int 0x10

	; reset floppy
	xor ah, ah
	xor dl, dl 
	int 0x13

	jmp $

BootMessage: db "Booting ..."

	times 510-($-$$)	db 0
	dw 0xaa55
	



