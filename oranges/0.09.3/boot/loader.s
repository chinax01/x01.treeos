; boot/loader.s 

	org 0x0100
	jmp start
%include "fat12.inc"
%include "load.inc"
%include "pm.inc"

gdts:	
	gdt_null:	Descriptor 0,0,0
	gdt_code:	Descriptor 0, 0xFFFFF, DA_CR | DA_32 | DA_LIMIT_4K
	gdt_data:	Descriptor 0, 0xFFFFF, DA_DRW | DA_32 | DA_LIMIT_4K
	gdt_video:	Descriptor 0xB8000, 0xFFFF, DA_DRW | DA_DPL3
GdtLen	equ $ - gdts 
gdt_ptr:	
	dw GdtLen - 1
	dd BaseOfLoaderPhyAddr + gdts 
SelectorCode	equ gdt_code - gdts 
SelectorData	equ gdt_data - gdts 
SelectorVideo	equ gdt_video - gdts + SA_RPL3

BaseOfStack 	equ 0x0100

start:
	mov ax, cs 
	mov ds, ax 
	mov es, ax
	mov ss, ax 
	mov sp, BaseOfStack

	mov dh, 0
	call DispStrRealMode

	; get memory number
	mov bx, 0
	mov di, _MemChkBuf
.MemChkLoop:
	mov eax, 0xE820
	mov ecx, 20
	mov edx, 0x534D4150
	int 0x15
	jc .MemChkFail
	add di, 20
	inc dword [_dwMCRNumber]
	cmp ebx, 0
	jne .MemChkLoop
	jmp .MemChkOK
.MemChkFail:
	mov dword [_dwMCRNumber], 0
.MemChkOK:


	; find kernel.bin 
	mov word [wSectorNo], SectorNoOfRootDirectory	
	xor ah, ah
	xor dl, dl 
	int 0x13	
search:
	cmp word [wRootDirSizeForLoop], 0
	jz no_kernel
	dec word [wRootDirSizeForLoop]
	mov ax, BaseOfKernelFile
	mov es, ax 
	mov bx, OffsetOfKernelFile
	mov ax, [wSectorNo]
	mov cl, 1
	call read_sector

	mov si, KernelFileName
	mov di, OffsetOfKernelFile
	cld 
	mov dx, 0x10 
for_kernel:
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
	mov si, KernelFileName
	jmp for_kernel
next_sector:
	add word [wSectorNo], 1
	jmp search
no_kernel:
	mov dh, 2
	call DispStrRealMode
	jmp $
filename_found:
	mov ax, RootDirSectors
	and di, 0xFFF0

	push eax
	mov eax, [es:di + 0x1C]
	mov dword [dwKernelSize], eax 
	pop eax 

	add di, 0x1A
	mov cx, word [es:di]	; fat firstclus
	push cx 
	add cx, ax 
	add cx, DeltaSectorNo
	mov ax, BaseOfKernelFile
	mov es, ax 
	mov bx, OffsetOfKernelFile
	mov ax, cx 
loading:
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
	jc .1
	jmp .2
.1:
	push ax 
	mov ax, es 
	add ax, 0x1000
	mov es, ax 
	pop ax 
.2:
	jmp loading
file_loaded:
	call kill_motor

	mov dh, 1
	call DispStrRealMode

	lgdt [gdt_ptr]
	cli 
	in al, 0x92
	or al, 2
	out 0x92, al 

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp dword SelectorCode:(BaseOfLoaderPhyAddr + s_code)

DispStrRealMode:
	mov ax, MessageLength
	mul dh
	add ax, LoadMessage
	mov bp, ax 
	mov ax, ds 
	mov es, ax 
	mov cx, MessageLength
	mov ax, 0x1301
	mov bx, 0x0007
	mov dl, 0
	add dh, 3
	int 0x10
	ret 

kill_motor:
	push dx 
	mov dx, 0x03F2
	mov al, 0
	out dx, al 
	pop dx 
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
.reading:
	mov ah, 2
	mov al, byte [bp-2]
	int 0x13
	jc .reading
	add esp, 2
	pop bp 
	ret 

get_fat:
	push es 
	push bx 
	push ax 
	mov ax, BaseOfKernelFile
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
dwKernelSize  dd 0

KernelFileName	db "KERNEL  BIN", 0
MessageLength	equ 9
LoadMessage:	db "Loading  "
Message1:		db "Ready.   "
Message2:		db "NO KERNEL"

[section .data]
align	32
[bits 32]
s_data:
; 实模式下使用这些符号
; 字符串
_szMemChkTitle:	db "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:	db "RAM size:", 0
_szReturn:	db 0Ah, 0
;; 变量
_dwMCRNumber:	dd 0	; Memory Check Result
_dwDispPos:	dd (80 * 6 + 0) * 2	; 屏幕第 6 行, 第 0 列。
_dwMemSize:	dd 0
_ARDStruct:	; Address Range Descriptor Structure
  _dwBaseAddrLow:		dd	0
  _dwBaseAddrHigh:		dd	0
  _dwLengthLow:			dd	0
  _dwLengthHigh:		dd	0
  _dwType:			dd	0
_MemChkBuf:	times	256	db	0
;
;; 保护模式下使用这些符号
szMemChkTitle		equ	BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize		equ	BaseOfLoaderPhyAddr + _szRAMSize
szReturn		equ	BaseOfLoaderPhyAddr + _szReturn
dwDispPos		equ	BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize		equ	BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber		equ	BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct		equ	BaseOfLoaderPhyAddr + _ARDStruct
	dwBaseAddrLow	equ	BaseOfLoaderPhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh	equ	BaseOfLoaderPhyAddr + _dwBaseAddrHigh
	dwLengthLow	equ	BaseOfLoaderPhyAddr + _dwLengthLow
	dwLengthHigh	equ	BaseOfLoaderPhyAddr + _dwLengthHigh
	dwType		equ	BaseOfLoaderPhyAddr + _dwType
MemChkBuf		equ	BaseOfLoaderPhyAddr + _MemChkBuf


; 堆栈就在数据段的末尾
StackSpace:	times	1024	db	0
TopOfStack	equ	BaseOfLoaderPhyAddr + $	; 栈顶

[section .code]
align 32
[bits 32]
s_code:
	mov ax, SelectorVideo
	mov gs, ax 

	mov ax, SelectorData 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov ss, ax 
	mov esp, TopOfStack

	call GetMemSize
	call SetupPaging
	call InitKernel

	; fill BOOT_PARAMS
	mov dword [BOOT_PARAM_ADDR], BOOT_PARAM_MAGIC
	mov eax, [dwMemSize]
	mov [BOOT_PARAM_ADDR + 4], eax 
	mov eax, BaseOfKernelFile
	shl eax, 4
	add eax, OffsetOfKernelFile
	mov [BOOT_PARAM_ADDR + 8], eax 

	jmp SelectorCode : KernelEntryPointPhyAddr

InitKernel:
	xor esi, esi 
	xor ecx, ecx 
	mov cx, word [BaseOfKernelFilePhyAddr + 0x2C]	;e_phnum
	mov esi, [BaseOfKernelFilePhyAddr + 0x1C]		;e_phoff
	add esi, BaseOfKernelFilePhyAddr
.begin:
	mov eax, [esi+0]
	cmp eax, 0
	jz .no_action
	push dword [esi + 0x10]		; p_filesz
	mov eax, [esi + 0x04]		; p_offset
	add eax, BaseOfKernelFilePhyAddr	; pSrc
	push eax 
	push dword [esi + 0x08]		; p_vaddr
	call MemCpy
	add esp, 12
.no_action:
	add esi, 0x20
	dec ecx 
	jnz .begin

	ret 


GetMemSize:
	push esi 
	push edi
	push ecx 

	mov esi, MemChkBuf
	mov ecx, [dwMCRNumber]
.loop:
	mov edx, 5
	mov edi, ARDStruct
.1:
	mov eax, dword [esi]
	stosd 
	add esi, 4
	dec edx 
	cmp edx, 0
	jnz .1
	cmp dword [dwType], 1
	jne .2 
	mov eax, [dwBaseAddrLow]
	add eax, [dwLengthLow]
	cmp eax, [dwMemSize]
	jb .2
	mov [dwMemSize], eax 
.2:
	loop .loop

	pop ecx 
	pop edi 
	pop esi 
	ret 

SetupPaging:
	xor edx, edx
	mov eax, [dwMemSize]
	mov ebx, 0x400000
	div ebx 
	mov ecx, eax 
	test edx, edx 
	jz .no_remainder
	inc ecx 
.no_remainder:
	push ecx 

	mov ax, SelectorData 
	mov es, ax 
	mov edi, PageDirBase
	xor eax, eax 
	mov eax, PageTblBase | PG_P | PG_USU | PG_RWW
.1:
	stosd 
	add eax, 4096
	loop .1

	pop eax
	mov ebx, 1024
	mul ebx 
	mov ecx, eax 
	mov edi, PageTblBase
	xor eax, eax 
	mov eax, PG_P | PG_USU | PG_RWW 
.2:
	stosd 
	add eax, 4096
	loop .2

	mov eax, PageDirBase
	mov cr3, eax 
	mov eax, cr0 
	or eax, 0x80000000
	mov cr0, eax 
	jmp short .3
.3:
	nop

	ret

%include "lib.inc"


