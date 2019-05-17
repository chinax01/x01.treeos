KernelEntryPointPhyAddr		equ 0x1000

BaseOfLoader	equ 0x9000
OffsetOfLoader	equ 0x0100
BaseOfLoaderPhyAddr		equ BaseOfLoader * 0x10

BaseOfKernelFile	equ 0x7000
OffsetOfKernelFile	equ 0
BaseOfKernelFilePhyAddr		equ BaseOfKernelFile * 0x10

PageDirBase 	equ 0x100000
PageTblBase		equ 0x101000

KERNEL_VALID_SPACE	equ BaseOfLoaderPhyAddr - BaseOfKernelFilePhyAddr

SUPER_BLK_SEG	equ 0x70

BOOT_PARAM_ADDR		equ 0x900
BOOT_PARAM_MAGIC	equ 0xB007

ROOT_BASE	equ 0x4EFF

;; corresponding with include/fs.h
SB_MAGIC_V1		equ	0x111
SB_MAGIC		equ	4 *  0
SB_NR_INODES		equ	4 *  1
SB_NR_SECTS		equ	4 *  2
SB_NR_IMAP_SECTS	equ	4 *  3
SB_NR_SMAP_SECTS	equ	4 *  4
SB_N_1ST_SECT		equ	4 *  5
SB_NR_INODE_SECTS	equ	4 *  6
SB_ROOT_INODE		equ	4 *  7
SB_INODE_SIZE		equ	4 *  8
SB_INODE_ISIZE_OFF	equ	4 *  9
SB_INODE_START_OFF	equ	4 * 10
SB_DIR_ENT_SIZE		equ	4 * 11
SB_DIR_ENT_INODE_OFF	equ	4 * 12
SB_DIR_ENT_FNAME_OFF	equ	4 * 13



