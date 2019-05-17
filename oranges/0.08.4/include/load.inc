BaseOfLoader	equ 0x9000
OffsetOfLoader	equ 0x0100
BaseOfLoaderPhyAddr		equ BaseOfLoader * 0x10

BaseOfKernelFile	equ 0x8000
OffsetOfKernelFile	equ 0
BaseOfKernelFilePhyAddr		equ BaseOfKernelFile * 0x10
KernelEntryPointPhyAddr		equ 0x30400

PageDirBase 	equ 0x200000
PageTblBase		equ 0x201000

