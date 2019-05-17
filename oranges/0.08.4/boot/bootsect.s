; boot/bootsect.s 2018 by x01

	org 0x7C00
	jmp short start
	nop
%include "fat12.inc"
%include "load.inc"

BaseOfStack equ 0x7c00

start:
	mov ax, cs 
	mov ds, ax 
	mov es, ax
	mov ss, ax 
	mov sp, BaseOfStack

	xor ah, ah
	xor dl, dl 
	int 0x13	

	mov word [wSectorNo], SectorNoOfRootDirectory	
search:
	cmp word [wRootDirSizeForLoop], 0
	jz no_loader
	dec word [wRootDirSizeForLoop]
	mov ax, BaseOfLoader
	mov es, ax 
	mov bx, OffsetOfLoader
	mov ax, [wSectorNo]
	mov cl, 1
	call read_sector

	mov si, LoaderFileName
	mov di, OffsetOfLoader
	cld 
	mov dx, 0x10 
for_loader:
	cmp dx, 8
	jz next_sector
	dec dx 
	mov cx, 11
cmp_filename:
	cmp cx, 0
	jz filename_found
	dec cx 
	lodsb 
	cmp al, [es:di]
	jz go_on
	jmp diff
go_on:
	inc di
	jmp cmp_filename
diff:
	and di, 0xFFE0
	add di, 0x20
	mov si, LoaderFileName
	jmp for_loader
next_sector:
	add word [wSectorNo], 1
	jmp search
no_loader:
	mov dh, 2
	call DispStr
	jmp $
filename_found:
	mov ax, RootDirSectors
	and di, 0xFFE0
	add di, 0x1A
	mov cx, word [es:di]	; fat firstclus
	push cx 
	add cx, ax 
	add cx, DeltaSectorNo
	mov ax, BaseOfLoader
	mov es, ax 
	mov bx, OffsetOfLoader
	mov ax, cx 
goon_loding_file:
	push ax
	push bx 
	mov ah, 0x0E
	mov al, '.'
	mov bx, 0x0F
	int 0x10
	pop bx 
	pop ax 

	mov cl, 1
	call read_sector
	pop ax 
	call get_fat
	cmp ax, 0xFFF
	jz file_loaded 
	push ax 
	mov dx, RootDirSectors
	add ax, dx 
	add ax, DeltaSectorNo
	add bx, [BPB_BytsPerSec]
	jmp goon_loding_file
file_loaded:
	; 清屏
	mov ax, 0x0600
	mov bx, 0x0700
	mov cx, 0
	mov dx, 0x184F
	int 0x10
	
	mov dh, 0
	call DispStr

	mov dh, 1
	call DispStr

	jmp BaseOfLoader:OffsetOfLoader

DispStr:
	mov ax, MessageLength
	mul dh
	add ax, BootMessage
	mov bp, ax 
	mov ax, ds 
	mov es, ax 
	mov cx, MessageLength
	mov ax, 0x1301
	mov bx, 0x0007
	mov dl, 0
	int 0x10
	ret 

read_sector:
	push bp 
	mov bp, sp 
	sub esp, 2

	mov byte [bp-2], cl 
	push bx 
	mov bl, [BPB_SecPerTrk]
	div bl 
	inc ah 
	mov cl, ah 
	mov dh, al 
	shr al, 1
	mov ch, al 
	and dh, 1
	pop bx 
	mov dl, [BS_DrvNum]
.goon_reading:
	mov ah, 2
	mov al, byte [bp-2]
	int 0x13
	jc .goon_reading
	add esp, 2
	pop bp 
	ret 

get_fat:
	push es 
	push bx 
	push ax 
	mov ax, BaseOfLoader
	sub ax, 0x0100
	mov es, ax 
	pop ax 
	mov byte [bOdd], 0
	mov bx, 3
	mul bx 
	mov bx, 2
	div bx 
	cmp dx, 0
	jz even
	mov byte [bOdd], 1
even:
	xor dx, dx
	mov bx, [BPB_BytsPerSec]
	div bx 
	push dx 
	mov bx, 0
	add ax, SectorNoOfFAT1
	mov cl, 2
	call read_sector
	pop dx 

	add bx, dx 
	mov ax, [es:bx]
	cmp byte [bOdd], 1
	jnz even2
	shr ax, 4
even2:
	and ax, 0x0FFF
get_fat_ok:
	pop bx 
	pop es 
	ret 

wRootDirSizeForLoop 	dw RootDirSectors
wSectorNo	dw 0
bOdd 		db 0
LoaderFileName	db "LOADER  BIN", 0
MessageLength	equ 9
BootMessage:	db "Booting  "
Message1:		db "Ready.   "
Message2:		db "NO LOADER"

times 510-($-$$) db 0
dw 0xAA55





