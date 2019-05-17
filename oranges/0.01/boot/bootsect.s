; boot/bootsect.s 2018 by x01

[BITS 16]
	org 0x7c00
	mov ax, cs
	mov ds, ax 
	mov es, ax 
	call print
	jmp $
print:
	mov ax, message
	mov bp, ax 
	mov cx, 14
	mov ax, 0x1301
	mov bx, 0x000c
	mov dl, 0
	int 0x10
	ret
message:
	db "hello OS-0.01!"

times 510-($-$$) db 0
db 0x55
db 0xAA
