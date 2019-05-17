; boot.s

	org 0x7c00

BaseOfStack equ 0x7c00
BaseOfLoader 	equ 0x1000
OffsetOfLoader 	equ 0x00

	jmp short _start
	nop 
%include "fat12.inc"

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

	; search loader.bin 
	mov word [SectorNo], SectorNumOfRootDirStart	
Label_Search_In_Root_Dir_Begin:
	cmp word [RootDirSizeForLoop], 0
	jz Label_No_LoaderBin
	dec word [RootDirSizeForLoop]
	mov ax, 0x00
	mov es, ax 
	mov bx, 0x8000
	mov ax, [SectorNo]
	mov cl, 1
	call Func_ReadOneSector
	mov si, LoaderFileName 
	mov di, 0x8000
	cld 
	mov dx, 0x10
Label_Search_For_LoaderBin:
	cmp	dx,	0
	jz	Label_Goto_Next_Sector_In_Root_Dir
	dec	dx
	mov	cx,	11
Label_Cmp_FileName:
	cmp	cx,	0
	jz	Label_FileName_Found
	dec	cx
	lodsb	
	cmp	al,	byte	[es:di]
	jz	Label_Go_On
	jmp	Label_Different
Label_Go_On:
	inc	di
	jmp	Label_Cmp_FileName
Label_Different:
	and	di,	0ffe0h
	add	di,	20h
	mov	si,	LoaderFileName
	jmp	Label_Search_For_LoaderBin
Label_Goto_Next_Sector_In_Root_Dir:
	add	word	[SectorNo],	1
	jmp	Label_Search_In_Root_Dir_Begin
Label_No_LoaderBin:
	mov	ax,	1301h
	mov	bx,	008ch
	mov	dx,	0100h
	mov	cx,	21
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	NoLoaderMessage
	int	10h
	jmp	$
Label_FileName_Found:
	mov	ax,	RootDirSectors
	and	di,	0ffe0h
	add	di,	01ah
	mov	cx,	word	[es:di]
	push	cx
	add	cx,	ax
	add	cx,	SectorBalance
	mov	ax,	BaseOfLoader
	mov	es,	ax
	mov	bx,	OffsetOfLoader
	mov	ax,	cx
Label_Go_On_Loading_File:
	push	ax
	push	bx
	mov	ah,	0eh
	mov	al,	'.'
	mov	bl,	0fh
	int	10h
	pop	bx
	pop	ax
	mov	cl,	1
	call	Func_ReadOneSector
	pop	ax
	call	Func_GetFATEntry
	cmp	ax,	0fffh
	jz	Label_File_Loaded
	push	ax
	mov	dx,	RootDirSectors
	add	ax,	dx
	add	ax,	SectorBalance
	add	bx,	[BPB_BytesPerSec]
	jmp	Label_Go_On_Loading_File
Label_File_Loaded:

	jmp BaseOfLoader:OffsetOfLoader

	; read one sector
Func_ReadOneSector:
	push	bp
	mov	bp,	sp
	sub	esp,	2
	mov	byte	[bp - 2],	cl
	push	bx
	mov	bl,	[BPB_SecPerTrk]
	div	bl
	inc	ah
	mov	cl,	ah
	mov	dh,	al
	shr	al,	1
	mov	ch,	al
	and	dh,	1
	pop	bx
	mov	dl,	[BS_DrvNum]
Label_Go_On_Reading:
	mov	ah,	2
	mov	al,	byte	[bp - 2]
	int	13h
	jc	Label_Go_On_Reading
	add	esp,	2
	pop	bp
	ret

	; get FAT entry
Func_GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax,	00
	mov	es,	ax
	pop	ax
	mov	byte	[Odd],	0
	mov	bx,	3
	mul	bx
	mov	bx,	2
	div	bx
	cmp	dx,	0
	jz	Label_Even
	mov	byte	[Odd],	1
Label_Even:
	xor	dx,	dx
	mov	bx,	[BPB_BytesPerSec]
	div	bx
	push	dx
	mov	bx,	8000h
	add	ax,	SectorNumOfFAT1Start
	mov	cl,	2
	call	Func_ReadOneSector
	pop	dx
	add	bx,	dx
	mov	ax,	[es:bx]
	cmp	byte	[Odd],	1
	jnz	Label_Even_2
	shr	ax,	4
Label_Even_2:
	and	ax,	0fffh
	pop	bx
	pop	es
	ret

;tmp variable
RootDirSizeForLoop	dw	RootDirSectors
SectorNo		dw	0
Odd			db	0

NoLoaderMessage:	db	"ERROR:No LOADER Found"
LoaderFileName:		db	"LOADER  BIN",0
BootMessage: db "Booting ..."

	times 510-($-$$)	db 0
	dw 0xaa55
	



